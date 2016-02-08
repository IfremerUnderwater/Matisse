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
#include <QFile>
#include <QDir>
#include "Polygon.h"
#include "RasterGeoreferencer.h"
#include "FileImgExposureCompensate.h"

using namespace std;
using namespace cv;
using namespace cv::detail;
using namespace basicproc;

MosaicDrawer::MosaicDrawer(QString drawingOptions)
{

    // Default command line args
    dOptions.tryGpu = false;
    dOptions.seamMegapix = 0.1;
    dOptions.exposCompType = ExposureCompensator::GAIN;
    dOptions.gainBlock = true;
    dOptions.seamFindType = "gc_color";
    dOptions.blendType = Blender::MULTI_BAND;
    dOptions.blendStrength = 1;

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
                dOptions.tryGpu = false;
            else if (argv[i + 1] == "yes")
                dOptions.tryGpu = true;
            else
            {
                qDebug()<< "Bad --try_gpu flag value\n";
                return -1;
            }
            i++;
        }
        else if (argv[i] == "--seam_megapix")
        {
            dOptions.seamMegapix = atof(argv[i + 1].toLocal8Bit().data());
            i++;
        }
        else if (argv[i] == "--expos_comp")
        {
            if (argv[i + 1] == "no"){
                dOptions.exposCompType = ExposureCompensator::NO;
                dOptions.gainBlock = false;
            }
            else if (argv[i + 1] == "gain"){
                dOptions.exposCompType = ExposureCompensator::GAIN;
                dOptions.gainBlock = false;
            }
            else if (argv[i + 1] == "gain_blocks"){
                dOptions.gainBlock = true;
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
                dOptions.seamFindType = argv[i + 1];
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
                dOptions.blendType = Blender::NO;
            else if (argv[i + 1] == "feather")
                dOptions.blendType = Blender::FEATHER;
            else if (argv[i + 1] == "multiband")
                dOptions.blendType = Blender::MULTI_BAND;
            else
            {
                qDebug()<< "Bad blending method\n";
                return -1;
            }
            i++;
        }
        else if (argv[i] == "--blend_strength")
        {
            dOptions.blendStrength = static_cast<float>(atof(argv[i + 1].toLocal8Bit().data()));
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

        if (dOptions.gainBlock){

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
            red_compensator = ExposureCompensator::createDefault(dOptions.exposCompType);
            green_compensator = ExposureCompensator::createDefault(dOptions.exposCompType);
            blue_compensator = ExposureCompensator::createDefault(dOptions.exposCompType);
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

        if (dOptions.gainBlock){

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
            compensator = ExposureCompensator::createDefault(dOptions.exposCompType);
        }

        compensator->feed(corners_p, imagesWarped_p, masksWarped_p);
    }

    // Convert images for seam processing *********************************************************************
    vector<Mat> imagesWarped_p_f(num_images);
    vector<Mat> masksWarped_p_seam(num_images);
    vector<Point> corners_p_seam(num_images);
    double seam_scale;

    seam_scale = min(1.0, sqrt(dOptions.seamMegapix * 1e6 / (double)(mean_row_nb*mean_col_nb)));
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
    qDebug()<< "Seam finder run..." <<dOptions.seamFindType;

    Ptr<SeamFinder> seam_finder;
    if (dOptions.seamFindType == "no")
        seam_finder = new detail::NoSeamFinder();
    else if (dOptions.seamFindType == "voronoi")
        seam_finder = new detail::VoronoiSeamFinder();
    else if (dOptions.seamFindType == "gc_color")
    {
#ifdef HAVE_OPENCV_GPU
        if (dOptions.tryGpu ){
            qDebug()<< "Computing with GPU\n";
            seam_finder = new detail::GraphCutSeamFinderGpu(GraphCutSeamFinderBase::COST_COLOR);
        }
        else
#endif
            seam_finder = new detail::GraphCutSeamFinder(GraphCutSeamFinderBase::COST_COLOR);
    }
    else if (dOptions.seamFindType == "gc_colorgrad")
    {
#ifdef HAVE_OPENCV_GPU
        if (dOptions.tryGpu )
            seam_finder = new detail::GraphCutSeamFinderGpu(GraphCutSeamFinderBase::COST_COLOR_GRAD);
        else
#endif
            seam_finder = new detail::GraphCutSeamFinder(GraphCutSeamFinderBase::COST_COLOR_GRAD);
    }
    else if (dOptions.seamFindType == "dp_color")
        seam_finder = new detail::DpSeamFinder(DpSeamFinder::COLOR);
    else if (dOptions.seamFindType == "dp_colorgrad")
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
    blender = Blender::createDefault(dOptions.blendType, dOptions.tryGpu);
    Size dst_sz = resultRoi(corners_p, sizes).size();
    float blend_width = sqrt(static_cast<float>(dst_sz.area())) * dOptions.blendStrength / 100.f;
    if (blend_width < 1.f)
        blender = Blender::createDefault(Blender::NO, dOptions.tryGpu);
    else if (dOptions.blendType == Blender::MULTI_BAND)
    {
        MultiBandBlender* mb = dynamic_cast<MultiBandBlender*>(static_cast<Blender*>(blender));
        mb->setNumBands(min(static_cast<int>(ceil(log(blend_width)/log(2.)) - 1.),(int)4));
        qDebug()<<  "Multi-band blender, number of bands: " << mb->numBands();
    }
    else if (dOptions.blendType == Blender::FEATHER)
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

void MosaicDrawer::blockDrawBlendAndWrite(const MosaicDescriptor &mosaicD_p, Point2d blockSize_p, QString writingPath_p, QString prefix_p)
{

    int xBlockNumber, yBlockNumber, xOverlapSize, yOverlapSize;
    int xLastColumnSize, yLastRowSize;

    //bool gainCompRequired = ( dOptions.exposCompType != ExposureCompensator::NO );

    // Create tmp folder to contain temporary files
    QDir dir(writingPath_p + QDir::separator() + QString("tmp"));
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // We first backup drawing options as we will change them on blocks blending iterations
    drawingOptions dOptionsBackup = dOptions;

    // 5% overlap hard coded for the moment
    xOverlapSize = (int)(0.05*blockSize_p.x);
    yOverlapSize = (int)(0.05*blockSize_p.y);

    // Get complete mosaic size
    Point2d mosaicSize = mosaicD_p.mosaicSize();

    // Compute last block size
    xLastColumnSize = (int)(mosaicSize.x) % ((int)blockSize_p.x-xOverlapSize);
    yLastRowSize = (int)(mosaicSize.y) % ((int)blockSize_p.y-yOverlapSize);

    // Deduce number of blocks
    if (xLastColumnSize == 0){
        xBlockNumber = mosaicSize.x / ((int)blockSize_p.x-xOverlapSize);
    }else{
        xBlockNumber = (mosaicSize.x - xLastColumnSize) / ((int)blockSize_p.x-xOverlapSize);
        xBlockNumber++;
    }

    if (yLastRowSize == 0){
        yBlockNumber = mosaicSize.y / ((int)blockSize_p.y-yOverlapSize);
    }else{
        yBlockNumber = (mosaicSize.y - yLastRowSize) / ((int)blockSize_p.y-yOverlapSize);
        yBlockNumber++;
    }

    // Allocate and build polygons associated with images
    vector<Polygon*> vpImagesPoly;


    for (int k=0; k < mosaicD_p.cameraNodes().size(); k++){
        std::vector<double> x,y;
        int xBegin, yBegin, xEnd, yEnd;
        cv::Point imgCorner;
        cv::Size imgSize;

        mosaicD_p.cameraNodes().at(k)->computeImageExtent(imgCorner, imgSize);

        // Compute extents
        xBegin = imgCorner.x;
        yBegin = imgCorner.y;

        xEnd = imgCorner.x + imgSize.width-1;
        yEnd = imgCorner.y + imgSize.height-1;

        // Construct currentPolygon
        Polygon *currentPolygon = new Polygon();
        x.push_back(xBegin); x.push_back(xEnd); x.push_back(xEnd); x.push_back(xBegin);
        y.push_back(yBegin); y.push_back(yBegin); y.push_back(yEnd); y.push_back(yEnd);
        currentPolygon->addContour(x,y);
        x.clear(); y.clear();
        vpImagesPoly.push_back(currentPolygon);

    }

    // Construct polygons corresponding to blocks areas
    vector<Polygon*> vpBlocksPoly;
    vector<Polygon*> vpEffBlocksPoly;
    vector<vector<int>*> vvBlocksImgIndexes;

    for (int j=0; j<xBlockNumber; j++){
        for (int i=0; i<yBlockNumber; i++){

            Polygon *currentBlockPolygon = new Polygon();
            Polygon *currentEffBlockPolygon = new Polygon();
            vector<int> *currentImgIndexes = new vector<int>;
            std::vector<double> x,y;
            int xBegin, yBegin, xEnd, yEnd;
            int xCurrentBlockSize, yCurrentBlockSize;

            // Handle last block size
            if (j == xBlockNumber-1){
                xCurrentBlockSize = xLastColumnSize;
            }else{
                xCurrentBlockSize = blockSize_p.x;
            }

            if (i == yBlockNumber-1){
                yCurrentBlockSize = yLastRowSize;
            }else{
                yCurrentBlockSize = blockSize_p.y;
            }

            // Compute extents
            xBegin = (blockSize_p.x-xOverlapSize)*j;
            yBegin = (blockSize_p.y-yOverlapSize)*i;

            xEnd = xBegin + xCurrentBlockSize-1;
            yEnd = yBegin + yCurrentBlockSize-1;

            // Construct currentPolygon
            x.push_back(xBegin); x.push_back(xEnd); x.push_back(xEnd); x.push_back(xBegin);
            y.push_back(yBegin); y.push_back(yBegin); y.push_back(yEnd); y.push_back(yEnd);
            currentBlockPolygon->addContour(x,y);
            x.clear(); y.clear();

            // Find which images intersect with this block
            // and compute effective block (ie. remove non occupied space)

            double img_tlx, img_tly, img_brx, img_bry;
            double block_tlx, block_tly, block_brx, block_bry;

            block_tlx=DBL_MAX;
            block_tly=DBL_MAX;
            block_brx=-DBL_MAX;
            block_bry=-DBL_MAX;

            for (int k=0; k < mosaicD_p.cameraNodes().size(); k++){

                Polygon *imgBlockIntersection = new Polygon;
                vpImagesPoly[k]->clip(*currentBlockPolygon,*imgBlockIntersection,basicproc::INT);

                if (!imgBlockIntersection->isEmpty()){

                    currentImgIndexes->push_back(k);
                    imgBlockIntersection->getBoundingBox(img_tlx, img_tly, img_brx, img_bry);

                    if (img_tlx < block_tlx)
                        block_tlx = img_tlx;

                    if (img_tly < block_tly)
                        block_tly = img_tly;

                    if (img_brx > block_brx)
                        block_brx = img_brx;

                    if (img_bry > block_bry)
                        block_bry = img_bry;

                }

                delete imgBlockIntersection;
            }

            if (currentImgIndexes->size() > 0){
                vpBlocksPoly.push_back(currentBlockPolygon);
                vvBlocksImgIndexes.push_back(currentImgIndexes);

                // Construct currentEffPolygon
                x.push_back(block_tlx); x.push_back(block_brx); x.push_back(block_brx); x.push_back(block_tlx);
                y.push_back(block_tly); y.push_back(block_tly); y.push_back(block_bry); y.push_back(block_bry);
                currentEffBlockPolygon->addContour(x,y);
                x.clear(); y.clear();
                vpEffBlocksPoly.push_back(currentEffBlockPolygon);
            }
            else{
                delete currentImgIndexes;
                delete currentBlockPolygon;
                delete currentEffBlockPolygon;
            }

        }
    }

    // Compensate all images gain at the same time if need (commented because using disk to store images is very slow)
    /*FileImgGainCompensator fileImgGainComp;

    if(gainCompRequired){

        int camNum = mosaicD_p.cameraNodes().size();

        QString infoFileName = writingPath_p + QDir::separator() + QString("tmp") + QDir::separator() + "proj_img.info";
        QString imageFilename;
        QString imageMaskFilename;
        QFile infoFile( infoFileName );

        Mat warpedImage;
        Mat warpedMask;
        Point corner;

        if ( !infoFile.open(QIODevice::ReadWrite) )
        {
            qFatal("You must specify a valid path for output info file...\n");
        }

        QTextStream stream( &infoFile );

        for (int l=0; l < camNum; l++) {

            ProjectiveCamera* Cam = mosaicD_p.cameraNodes().at(l);
            Cam->projectImageOnMosaickingPlane(warpedImage, warpedMask, corner);

            imageFilename = writingPath_p + QDir::separator() + QString("tmp") + QDir::separator() + QString("projimg_%1.tiff").arg(l, 5, 'g', -1, '0');
            imageMaskFilename = writingPath_p + QDir::separator() + QString("tmp") + QDir::separator() + QString("projimg_mask_%1.tiff").arg(l, 5, 'g', -1, '0');

            imwrite(imageFilename.toStdString().c_str(), warpedImage);
            imwrite(imageMaskFilename.toStdString().c_str(), warpedMask);

            stream << QString("projimg_%1.tiff").arg(l, 5, 'g', -1, '0')
                   << ";" << QString("projimg_mask_%1.tiff").arg(l, 5, 'g', -1, '0')
                   << ";" << corner.x << ";" << corner.y
                   << ";" << warpedImage.cols << ";" << warpedImage.rows << endl;

            Cam->image()->releaseImageData();

        }

        infoFile.close();

        fileImgGainComp.feed(writingPath_p + QDir::separator() + QString("tmp"),QString("proj_img.info"));

    }*/


    // Blend blocks independantly
    for (unsigned int k=0; k < vpEffBlocksPoly.size(); k++){
        std::vector<Mat> imagesWarped;
        std::vector<Mat> masksWarped;
        std::vector<Point> corners;
        Mat mosaicImage, mosaicImageMask;

        int camNum = vvBlocksImgIndexes[k]->size();

        imagesWarped.resize(camNum);
        masksWarped.resize(camNum);
        corners.resize(camNum);

        // Project each image on the mosaicking plane (TODO : make it generic for orthophoto 3D/2D mosaicking)
        for (int l=0; l < camNum; l++) {

            Polygon *imgBlockInter = new Polygon();
            vpImagesPoly[vvBlocksImgIndexes[k]->at(l)]->clip(*(vpEffBlocksPoly[k]),*imgBlockInter,basicproc::INT);

            ProjectiveCamera* Cam = mosaicD_p.cameraNodes().at(vvBlocksImgIndexes[k]->at(l));

            if (*(vpImagesPoly[vvBlocksImgIndexes[k]->at(l)]) == *imgBlockInter){
                Cam->projectImageOnMosaickingPlane(imagesWarped[l], masksWarped[l], corners[l]);

            }else{
                Mat tempImgWarped,tempMaskWarped;
                Cam->projectImageOnMosaickingPlane(tempImgWarped, tempMaskWarped, corners[l]);

                // Get useful image part
                double tl_x,tl_y,br_x,br_y;
                imgBlockInter->getBoundingBox(tl_x,tl_y,br_x,br_y);

                // Crop images and masks with the bounding box
                tempImgWarped(cv::Rect(tl_x-corners[l].x,tl_y-corners[l].y,br_x-tl_x+1,br_y-tl_y+1)).copyTo(imagesWarped[l]);
                tempMaskWarped(cv::Rect(tl_x-corners[l].x,tl_y-corners[l].y,br_x-tl_x+1,br_y-tl_y+1)).copyTo(masksWarped[l]);
                corners[l].x = tl_x;
                corners[l].y = tl_y;
            }

            Cam->image()->releaseImageData();
            delete imgBlockInter;

        }

        // Draw block
        drawAndBlend(imagesWarped, masksWarped, corners, mosaicImage, mosaicImageMask);
        QString mosaicFilePath = writingPath_p + QDir::separator() + QString("tmp")
                + QDir::separator() + prefix_p + QString("_temp%1.tiff").arg(k, 4, 'g', -1, '0');
        imwrite(mosaicFilePath.toStdString().c_str(), mosaicImage);

        QString mosaicMaskFilePath = writingPath_p + QDir::separator() + QString("tmp")
                + QDir::separator() + prefix_p + QString("_masktemp%1.tiff").arg(k, 4, 'g', -1, '0');
        imwrite(mosaicMaskFilePath.toStdString().c_str(), mosaicImageMask);

        /*QString utmProjParam, utmHemisphereOption,utmZoneString;

        // Construct utm proj param options
        utmZoneString = QString("%1 ").arg(mosaicD_p.utmZone())+ mosaicD_p.utmHemisphere();
        QStringList utmParams = utmZoneString.split(" ");

        if ( utmParams.at(1) == "S" ){
            utmHemisphereOption = QString(" +south");
        }else{
            utmHemisphereOption = QString("");
        }
        utmProjParam = QString("+proj=utm +zone=") + utmParams.at(0);

        // Get block corners in pixels
        double blockTL_x,blockTL_y,blockBR_x,blockBR_y;
        vpEffBlocksPoly[k]->getBoundingBox(blockTL_x,blockTL_y,blockBR_x,blockBR_y);


        double blockUtmTL_x = mosaicD_p.mosaicOrigin().x + blockTL_x*mosaicD_p.pixelSize().x;
        double blockUtmTL_y = mosaicD_p.mosaicOrigin().y - blockTL_y*mosaicD_p.pixelSize().y;

        double blockUtmBR_x = mosaicD_p.mosaicOrigin().x + blockBR_x*mosaicD_p.pixelSize().x;
        double blockUtmBR_y = mosaicD_p.mosaicOrigin().y - blockBR_y*mosaicD_p.pixelSize().y;

        QString gdalOptions =  QString("-a_srs \"")+ utmProjParam + QString("\" -of GTiff -co \"INTERLEAVE=PIXEL\" -a_ullr %1 %2 %3 %4")
                .arg(blockUtmTL_x,0,'f',2)
                .arg(blockUtmTL_y,0,'f',2)
                .arg(blockUtmBR_x,0,'f',2)
                .arg(blockUtmBR_y,0,'f',2);
        RasterGeoreferencer rasterGeoref;
        rasterGeoref.WriteGeoFile(mosaicImage, mosaicImageMask, filePath,gdalOptions);*/

    }


    // Blend junction between pairs of blocks
    vector<Polygon*> vpBlocksPairIntersectPoly;

    // Adapt settings to junction blending
    dOptions.seamMegapix = 0.1;
    dOptions.exposCompType = ExposureCompensator::NO;
    dOptions.gainBlock = false;
    dOptions.seamFindType = "gc_color";
    dOptions.blendType = Blender::FEATHER;
    dOptions.blendStrength = 1;

    for (unsigned int k=0; k < vpEffBlocksPoly.size()-1; k++){

        std::vector<Mat> blocksToBeBlended;
        std::vector<Mat> blocksToBeBlendedMasks;
        std::vector<Point> corners;
        Mat blendedBlocksImg, blendedBlocksImgMask;

        blocksToBeBlended.resize(2);
        blocksToBeBlendedMasks.resize(2);
        corners.resize(2);

        QString imgFilePath1,imgFilePath2;
        QString mosaicMaskFilePath1,mosaicMaskFilePath2;

        for (unsigned int l=k+1; l<vpEffBlocksPoly.size(); l++){

            Polygon *blocksPairInter = new Polygon();
            vpEffBlocksPoly[k]->clip(*(vpEffBlocksPoly[l]),*blocksPairInter,basicproc::INT);
            double tl_x1,tl_y1,br_x1,br_y1;
            double tl_x2,tl_y2,br_x2,br_y2;

            if( !(blocksPairInter->isEmpty()) ){

                // Open first block and mask & get corner
                imgFilePath1 = writingPath_p + QDir::separator() + QString("tmp") + QDir::separator() + prefix_p + QString("_temp%1.tiff").arg(k, 4, 'g', -1, '0');
                blocksToBeBlended[0] = imread(imgFilePath1.toStdString().c_str());

                mosaicMaskFilePath1 = writingPath_p + QDir::separator() + QString("tmp") + QDir::separator() + prefix_p + QString("_masktemp%1.tiff").arg(k, 4, 'g', -1, '0');
                blocksToBeBlendedMasks[0] = imread(mosaicMaskFilePath1.toStdString().c_str(),CV_LOAD_IMAGE_GRAYSCALE);

                vpEffBlocksPoly[k]->getBoundingBox(tl_x1,tl_y1,br_x1,br_y1);
                corners[0].x = (int) tl_x1;
                corners[0].y = (int) tl_y1;

                // Open second block and mask
                imgFilePath2 = writingPath_p + QDir::separator() + QString("tmp") + QDir::separator() + prefix_p + QString("_temp%1.tiff").arg(l, 4, 'g', -1, '0');
                blocksToBeBlended[1] = imread(imgFilePath2.toStdString().c_str());

                mosaicMaskFilePath2 = writingPath_p + QDir::separator() + QString("tmp") + QDir::separator() + prefix_p + QString("_masktemp%1.tiff").arg(l, 4, 'g', -1, '0');
                blocksToBeBlendedMasks[1] = imread(mosaicMaskFilePath2.toStdString().c_str(),CV_LOAD_IMAGE_GRAYSCALE);

                vpEffBlocksPoly[l]->getBoundingBox(tl_x2,tl_y2,br_x2,br_y2);
                corners[1].x = (int) tl_x2;
                corners[1].y = (int) tl_y2;

                // Blend blocks
                drawAndBlend(blocksToBeBlended, blocksToBeBlendedMasks, corners, blendedBlocksImg, blendedBlocksImgMask);

                // Extract and write block 1
                int roi_x1,roi_x2,roi_y1,roi_y2;

                roi_x1 = tl_x1 - std::min(tl_x1, tl_x2);
                roi_x2 = br_x1 - std::min(tl_x1, tl_x2);
                roi_y1 = tl_y1 - std::min(tl_y1, tl_y2);
                roi_y2 = br_y1 - std::min(tl_y1, tl_y2);

                imwrite(imgFilePath1.toStdString().c_str(),blendedBlocksImg(cv::Rect(roi_x1,roi_y1,roi_x2-roi_x1+1,roi_y2-roi_y1+1)));
                imwrite(mosaicMaskFilePath1.toStdString().c_str(),blendedBlocksImgMask(cv::Rect(roi_x1,roi_y1,roi_x2-roi_x1+1,roi_y2-roi_y1+1)));

                // Extract and write block 2

                roi_x1 = tl_x2 - std::min(tl_x1, tl_x2);
                roi_x2 = br_x2 - std::min(tl_x1, tl_x2);
                roi_y1 = tl_y2 - std::min(tl_y1, tl_y2);
                roi_y2 = br_y2 - std::min(tl_y1, tl_y2);

                imwrite(imgFilePath2.toStdString().c_str(),blendedBlocksImg(cv::Rect(roi_x1,roi_y1,roi_x2-roi_x1+1,roi_y2-roi_y1+1)));
                imwrite(mosaicMaskFilePath2.toStdString().c_str(),blendedBlocksImgMask(cv::Rect(roi_x1,roi_y1,roi_x2-roi_x1+1,roi_y2-roi_y1+1)));

                vpBlocksPairIntersectPoly.push_back(blocksPairInter);


            }else{
                delete blocksPairInter;
            }



        }


    }


    // Blend intersection between junctions
    Polygon tempPoly1, tempPoly2, junctionInterPoly;

    if (vpBlocksPairIntersectPoly.size()!=0){

        for (unsigned int i=0; i<vpBlocksPairIntersectPoly.size()-1; i++){

            for (unsigned int j=i+1; i<vpBlocksPairIntersectPoly.size(); i++){

                // Intersect junctions
                vpBlocksPairIntersectPoly[i]->clip(*(vpBlocksPairIntersectPoly[j]),tempPoly1,basicproc::INT);

                // Add intersection
                junctionInterPoly.clip(tempPoly1,tempPoly2,basicproc::UNION);

                // Affect
                junctionInterPoly = tempPoly2;

            }

        }

    }

    // Restore drawing options
    dOptions = dOptionsBackup;

    // Free memory
    for (unsigned int i=0; i<vpBlocksPoly.size(); i++){
        delete vpBlocksPoly.at(i);
    }

    for (unsigned int i=0; i<vpEffBlocksPoly.size(); i++){
        delete vpEffBlocksPoly.at(i);
    }

    for (unsigned int i=0; i<vpImagesPoly.size(); i++){
        delete vpImagesPoly.at(i);
    }

    for (unsigned int i=0; i<vpBlocksPairIntersectPoly.size(); i++){
        delete vpBlocksPairIntersectPoly.at(i);
    }

}
