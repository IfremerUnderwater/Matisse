#include "MosaicDrawer.h"
#include "opencv2/opencv_modules.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/stitching/detail/autocalib.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"
#include "opencv2/stitching/detail/util.hpp"
#include "opencv2/core/core.hpp"
#include <QDebug>

using namespace std;
using namespace cv;
using namespace cv::detail;

MosaicDrawer::MosaicDrawer(QString drawingOptions)
{

    // Default command line args
    _tryGpu = false;
    _seamMegapix = 0.1;
    _exposCompType;
    _gainBlock = true;
    _seamFindType = "gc_color";
    _blendType = Blender::MULTI_BAND;
    _blendStrength = 1;

    this->parseAndAffectOptions( drawingOptions );
}


int MosaicDrawer::parseAndAffectOptions(QString drawingOptions)
{

    QStringList argv = drawingOptions.split(" ");
    int argc = argv.size();

    for (int i = 0; i < argc; ++i)
    {
        if (argv[i] == "--try_gpu")
        {
            if (argv[i + 1] == "no")
                _tryGpu = false;
            else if (argv[i + 1] == "yes")
                _tryGpu = true;
            else
            {
                qDebug()<< "Bad --try_gpu flag value\n";
                return -1;
            }
            i++;
        }
        else if (argv[i] == "--seam_megapix")
        {
            _seamMegapix = atof(argv[i + 1].toLocal8Bit().data());
            i++;
        }
        else if (argv[i] == "--expos_comp")
        {
            if (argv[i + 1] == "no"){
                _exposCompType = ExposureCompensator::NO;
                _gainBlock = false;
            }
            else if (argv[i + 1] == "gain"){
                _exposCompType = ExposureCompensator::GAIN;
                _gainBlock = false;
            }
            else if (argv[i + 1] == "gain_blocks"){
                _gainBlock = true;
            }
            else
            {
                qDebug()<< "Bad exposure compensation method\n";
                return -1;
            }
            i++;
        }
        else if (argv[i] == "--seam")
        {
            if (argv[i + 1] == "no" ||
                    argv[i + 1] == "voronoi" ||
                    argv[i + 1] == "gc_color" ||
                    argv[i + 1] == "gc_colorgrad" ||
                    argv[i + 1] == "dp_color" ||
                    argv[i + 1] == "dp_colorgrad")
                _seamFindType = argv[i + 1];
            else
            {
                qDebug()<< "Bad seam finding method\n";
                return -1;
            }
            i++;
        }
        else if (argv[i] == "--blend")
        {
            if (argv[i + 1] == "no")
                _blendType = Blender::NO;
            else if (argv[i + 1] == "feather")
                _blendType = Blender::FEATHER;
            else if (argv[i + 1] == "multiband")
                _blendType = Blender::MULTI_BAND;
            else
            {
                qDebug()<< "Bad blending method\n";
                return -1;
            }
            i++;
        }
        else if (argv[i] == "--blend_strength")
        {
            _blendStrength = static_cast<float>(atof(argv[i + 1].toLocal8Bit().data()));
            i++;
        }
    }

    return 0;
}

void MosaicDrawer::drawAndBlend(const MosaicDescriptor &mosaicD_p, Mat &mosaicImage_p, Mat &mosaicImageMask_p)
{

    std::vector<Mat> imagesWarped;
    std::vector<Mat> masksWarped;
    std::vector<Point> corners;

    int camNum = mosaicD_p.cameraNodes().size();

    imagesWarped.resize(camNum);
    masksWarped.resize(camNum);
    corners.resize(camNum);

    int i=0;

    // Project each image on the mosaicking plane
    foreach (ProjectiveCamera* Cam, mosaicD_p.cameraNodes()) {

        Cam->projectImageOnMosaickingPlane(imagesWarped[i], masksWarped[i], corners[i]);
        Cam->image()->releaseImageData();
        i++;
    }

    drawAndBlend(imagesWarped, masksWarped, corners, mosaicImage_p, mosaicImageMask_p);

}

void MosaicDrawer::drawAndBlend(std::vector<Mat> &imagesWarped_p, std::vector<Mat> &masksWarped_p, std::vector<Point> &corners_p, Mat &mosaicImage_p, Mat &mosaicImageMask_p)
{

    bool colored_images = true;

    int num_images = static_cast<int>(imagesWarped_p.size());
    vector<Size> sizes(num_images);

    if (imagesWarped_p[0].channels() == 3){
        colored_images = true;
    }else{
        colored_images = false;
    }

    qDebug()<< "There are "<< num_images <<" images to process\n\n";

    // Store images size in sizes vector
    int mean_col_nb = 0;
    int mean_row_nb = 0;

    if (colored_images){// Process 3 Channels separatly

        // Init Separated channels
        vector<Mat> red_images(num_images);
        vector<Mat> green_images(num_images);
        vector<Mat> blue_images(num_images);
        vector<Mat> TempRGB(3);

        for (int i=0; i<num_images; i++){

            sizes[i] = imagesWarped_p[i].size();

            // Split channels
            split(imagesWarped_p[i], TempRGB);
            red_images[i] = TempRGB[0].clone();
            green_images[i] = TempRGB[1].clone();
            blue_images[i] = TempRGB[2].clone();

            mean_col_nb += imagesWarped_p[i].cols;
            mean_row_nb += imagesWarped_p[i].rows;

        }

        mean_col_nb /= num_images;
        mean_row_nb /= num_images;

        //printf("Mean mega_pixel size %.2f...\n",(double)(mean_row_nb*mean_col_nb)/1e6);

        qDebug()<< "Compensating exposure... \n";

        // Create image compensator per channel
        double Bl_per_image;
        int Bl_num_col,Bl_num_row,Bl_w,Bl_h;
        Ptr<ExposureCompensator> red_compensator;
        Ptr<ExposureCompensator> green_compensator;
        Ptr<ExposureCompensator> blue_compensator;

        if (_gainBlock){

            Bl_per_image = sqrt(10e9/(100*imagesWarped_p.size()*imagesWarped_p.size())); // 10e9 is for 1Giga and 100 is due to the number of allocations
            Bl_num_col = ceil(sqrt(Bl_per_image*(double)mean_row_nb/(double)mean_col_nb));
            Bl_num_row = ceil(Bl_num_col*(double)mean_col_nb/(double)mean_row_nb);

            Bl_w = (mean_col_nb-1)/(Bl_num_col-1);
            Bl_h = (mean_row_nb-1)/(Bl_num_row-1);

            Bl_w = max(Bl_w, 50);
            Bl_h = max(Bl_h, 50);

            qDebug()<< "Blocks Gain Compensator with Bl_w = "<< Bl_w << "Bl_h = "<< Bl_h <<"\n";

            red_compensator = new BlocksGainCompensator(Bl_w,Bl_h);
            green_compensator = new BlocksGainCompensator(Bl_w,Bl_h);
            blue_compensator = new BlocksGainCompensator(Bl_w,Bl_h);

        }else{
            qDebug()<< "Create default Compensator ...\n";
            red_compensator = ExposureCompensator::createDefault(_exposCompType);
            green_compensator = ExposureCompensator::createDefault(_exposCompType);
            blue_compensator = ExposureCompensator::createDefault(_exposCompType);
        }

        //(Do not remove the following three loops, they are needed to minimize memory use)
        // Process exposure compensation for the red channel

        for (int i = 0; i < num_images; ++i){

            cvtColor(red_images[i], red_images[i], CV_GRAY2BGR);

        }
        red_compensator->feed(corners_p, red_images, masksWarped_p);
        red_images.clear();

        // Process exposure compensation for the green channel
        for (int i = 0; i < num_images; ++i){

            cvtColor(green_images[i], green_images[i], CV_GRAY2BGR);

        }
        green_compensator->feed(corners_p, green_images, masksWarped_p);
        green_images.clear();

        // Process exposure compensation for the blue channel
        for (int i = 0; i < num_images; ++i){

            cvtColor(blue_images[i], blue_images[i], CV_GRAY2BGR);

        }
        blue_compensator->feed(corners_p, blue_images, masksWarped_p);
        blue_images.clear();

        // Compensate exposure
        for (int img_idx = 0; img_idx < num_images; ++img_idx){
            split(imagesWarped_p[img_idx], TempRGB);
            cvtColor(TempRGB[0], TempRGB[0], CV_GRAY2BGR);
            cvtColor(TempRGB[1], TempRGB[1], CV_GRAY2BGR);
            cvtColor(TempRGB[2], TempRGB[2], CV_GRAY2BGR);

            red_compensator->apply(img_idx, corners_p[img_idx], TempRGB[0], masksWarped_p[img_idx]);
            green_compensator->apply(img_idx, corners_p[img_idx], TempRGB[1], masksWarped_p[img_idx]);
            blue_compensator->apply(img_idx, corners_p[img_idx], TempRGB[2], masksWarped_p[img_idx]);

            cvtColor(TempRGB[0], TempRGB[0], CV_BGR2GRAY);
            cvtColor(TempRGB[1], TempRGB[1], CV_BGR2GRAY);
            cvtColor(TempRGB[2], TempRGB[2], CV_BGR2GRAY);

            merge(TempRGB,imagesWarped_p[img_idx]);
        }

    }else{ // Process B&W Images (compensation)

        for (int i=0; i<num_images; i++){

            sizes[i] = imagesWarped_p[i].size();
            cvtColor(imagesWarped_p[i], imagesWarped_p[i], CV_GRAY2BGR);

            mean_col_nb += imagesWarped_p[i].cols;
            mean_row_nb += imagesWarped_p[i].rows;

        }

        mean_col_nb /= num_images;
        mean_row_nb /= num_images;

        // Convert images for seam and exposure processing
        vector<Mat> imagesWarped_p_f(num_images);

        for (int i = 0; i < num_images; ++i){

            imagesWarped_p[i].convertTo(imagesWarped_p_f[i], CV_32F);

        }

        // Create image compensator
        double Bl_per_image;
        int Bl_num_col,Bl_num_row,Bl_w,Bl_h;
        Ptr<ExposureCompensator> compensator;

        if (_gainBlock){

            Bl_per_image = sqrt(10e9/(100*imagesWarped_p.size()*imagesWarped_p.size())); // 10e9 is for 1Giga and 100 is due to the number of allocations
            Bl_num_col = ceil(sqrt(Bl_per_image*(double)mean_row_nb/(double)mean_col_nb));
            Bl_num_row = ceil(Bl_num_col*(double)mean_col_nb/(double)mean_row_nb);

            Bl_w = (mean_col_nb-1)/(Bl_num_col-1);
            Bl_h = (mean_row_nb-1)/(Bl_num_row-1);

            Bl_w = max(Bl_w, 50);
            Bl_h = max(Bl_h, 50);

            qDebug()<< "Blocks Gain Compensator with Bl_w = "<< Bl_w << "Bl_h = "<< Bl_h <<"\n";

            compensator = new BlocksGainCompensator(Bl_w,Bl_h);
        }else{
            compensator = ExposureCompensator::createDefault(_exposCompType);
        }

        compensator->feed(corners_p, imagesWarped_p, masksWarped_p);
    }

    // Convert images for seam processing *********************************************************************
    vector<Mat> imagesWarped_p_f(num_images);
    vector<Mat> masksWarped_p_seam(num_images);
    vector<Point> corners_p_seam(num_images);
    double seam_scale;

    seam_scale = min(1.0, sqrt(_seamMegapix * 1e6 / (double)(mean_row_nb*mean_col_nb)));
    qDebug()<< "Theoric seam_scale = " << seam_scale <<"\n";

    for (int i = 0; i < num_images; ++i){

        if (seam_scale < 1){

            // resize image to reduce seam finding computing time
            resize(imagesWarped_p[i], imagesWarped_p_f[i], Size(), seam_scale, seam_scale);
            imagesWarped_p_f[i].convertTo(imagesWarped_p_f[i], CV_32F);

            // same for the mask
            resize(masksWarped_p[i],masksWarped_p_seam[i], Size(), seam_scale, seam_scale);

            // same for the corners_p
            corners_p_seam[i] = seam_scale*corners_p[i];

        }else{

            imagesWarped_p[i].convertTo(imagesWarped_p_f[i], CV_32F);
        }

    }

    // Run the seam finder ...
    qDebug()<< "Seam finder run..." <<_seamFindType;

    Ptr<SeamFinder> seam_finder;
    if (_seamFindType == "no")
        seam_finder = new detail::NoSeamFinder();
    else if (_seamFindType == "voronoi")
        seam_finder = new detail::VoronoiSeamFinder();
    else if (_seamFindType == "gc_color")
    {
#ifdef HAVE_OPENCV_GPU
        if (_tryGpu ){
            qDebug()<< "Computing with GPU\n";
            seam_finder = new detail::GraphCutSeamFinderGpu(GraphCutSeamFinderBase::COST_COLOR);
        }
        else
#endif
            seam_finder = new detail::GraphCutSeamFinder(GraphCutSeamFinderBase::COST_COLOR);
    }
    else if (_seamFindType == "gc_colorgrad")
    {
#ifdef HAVE_OPENCV_GPU
        if (_tryGpu )
            seam_finder = new detail::GraphCutSeamFinderGpu(GraphCutSeamFinderBase::COST_COLOR_GRAD);
        else
#endif
            seam_finder = new detail::GraphCutSeamFinder(GraphCutSeamFinderBase::COST_COLOR_GRAD);
    }
    else if (_seamFindType == "dp_color")
        seam_finder = new detail::DpSeamFinder(DpSeamFinder::COLOR);
    else if (_seamFindType == "dp_colorgrad")
        seam_finder = new detail::DpSeamFinder(DpSeamFinder::COLOR_GRAD);
    if (seam_finder.empty())
    {
        qDebug()<< "Can't create the following seam finder (use the default one instead)\n";
    }

    // Run the seam finder
    if (seam_scale < 1){
        seam_finder->find(imagesWarped_p_f, corners_p_seam, masksWarped_p_seam);
    }else{
        seam_finder->find(imagesWarped_p_f, corners_p, masksWarped_p);
    }


    // Release unused images
    corners_p_seam.clear();
    imagesWarped_p_f.clear();

    qDebug()<< "Compositing...\n";

#if ENABLE_LOG
    t = getTickCount();
#endif

    Mat img_warped_s;
    Mat dilated_mask, seam_mask, mask_warped;
    Ptr<Blender> blender;

    // Initialize the blender
    blender = Blender::createDefault(_blendType, _tryGpu);
    Size dst_sz = resultRoi(corners_p, sizes).size();
    float blend_width = sqrt(static_cast<float>(dst_sz.area())) * _blendStrength / 100.f;
    if (blend_width < 1.f)
        blender = Blender::createDefault(Blender::NO, _tryGpu);
    else if (_blendType == Blender::MULTI_BAND)
    {
        MultiBandBlender* mb = dynamic_cast<MultiBandBlender*>(static_cast<Blender*>(blender));
        mb->setNumBands(min(static_cast<int>(ceil(log(blend_width)/log(2.)) - 1.),(int)4));
        qDebug()<<  "Multi-band blender, number of bands: " << mb->numBands();
    }
    else if (_blendType == Blender::FEATHER)
    {
        FeatherBlender* fb = dynamic_cast<FeatherBlender*>(static_cast<Blender*>(blender));
        fb->setSharpness(1.f/blend_width);
        qDebug()<< "Feather blender, sharpness: "<< fb->sharpness();
    }
    blender->prepare(corners_p, sizes);

    for (int img_idx = 0; img_idx < num_images; ++img_idx)
    {

        imagesWarped_p[img_idx].convertTo(img_warped_s, CV_16S);

        if (seam_scale < 1){
            // Upscale the seaming mask
            dilate(masksWarped_p_seam[img_idx], dilated_mask, Mat());
            //dilate(masksWarped_p[img_idx], dilated_mask, Mat());
            resize(dilated_mask, seam_mask, masksWarped_p[img_idx].size());
            mask_warped = seam_mask & masksWarped_p[img_idx];

            // Blend the current image
            blender->feed(img_warped_s, mask_warped, corners_p[img_idx]);
        }else{
            blender->feed(img_warped_s, masksWarped_p[img_idx], corners_p[img_idx]);
        }


    }

    qDebug()<< "Blend...";

    blender->blend(mosaicImage_p, mosaicImageMask_p);

    LOGLN("Compositing, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec");

    LOGLN("Finished, total time: " << ((getTickCount() - app_start_time) / getTickFrequency()) << " sec");

    // Convert result to 8U
    mosaicImage_p.convertTo(mosaicImage_p,CV_8U);

}
