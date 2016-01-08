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
#include "Polygon.h"
#include "RasterGeoreferencer.h"

using namespace std;
using namespace cv;
using namespace cv::detail;
using namespace basicproc;


MosaicDrawer::MosaicDrawer(QString drawingOptions)
{

    // Default command line args
    _tryGpu = false;
    _seamMegapix = 0.1;
    _exposCompType = ExposureCompensator::GAIN;
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

void MosaicDrawer::blockDrawBlendAndWrite(const MosaicDescriptor &mosaicD_p, Point2d blockSize_p, QString writingPathAndPrefix_p)
{

    int xBlockNumber, yBlockNumber, xOverlapSize, yOverlapSize;
    int xLastColumnSize, yLastRowSize;

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

    // Blend separated modules
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
        QString filePath = writingPathAndPrefix_p + QString("_temp%1.tiff").arg(k, 4, 'g', -1, '0');
        //imwrite(filePath.toStdString().c_str(), mosaicImage);

        QString utmProjParam, utmHemisphereOption,utmZoneString;

        // Construct utm proj param options
        utmZoneString = QString("%1 ").arg(mosaicD_p.utmZone())+ mosaicD_p.utmHemisphere();
        QStringList utmParams = utmZoneString.split(" ");

        if ( utmParams.at(1) == "S" ){
            utmHemisphereOption = QString(" +south");
        }else{
            utmHemisphereOption = QString("");
        }
        utmProjParam = QString("+proj=utm +zone=") + utmParams.at(0);

//        _mosaicOrigin.x, _mosaicOrigin.y,
//                            _mosaicOrigin.x+x_shift*_pixelSize.x, _mosaicOrigin.y-y_shift*_pixelSize.y

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
        rasterGeoref.WriteGeoFile(mosaicImage, mosaicImageMask, filePath,gdalOptions);

    }

    //    for m=1:x_bl_nb*y_bl_nb

    //        multiWaitbar( 'Blocks blending progress', 'Value', m/(x_bl_nb*y_bl_nb));

    //        if (~isempty(M_blocks_indexes{m}))
    //            M_blocks{m}.init = M.init;
    //            M_blocks{m}.nodes = M.nodes(M_blocks_indexes{m});
    //            [BlendedMosaic, BlendedMosaic_mask, P_eff_blocks{m}] = mosaic_draw_and_blend(M_blocks{m},algo_param);
    //            plot([P_eff_blocks{m}.x P_eff_blocks{m}.x(1)], [P_eff_blocks{m}.y P_eff_blocks{m}.y(1)], 'r');
    //            pause(eps)


    //            % Save mosaic to free memory
    //            BlendedMosaic(:,:,size(BlendedMosaic,3)+1) = BlendedMosaic_mask;
    //            write_tiff_alpha(BlendedMosaic,fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m)))
    //            clear BlendedMosaic BlendedMosaic_mask

    //            % Update block size and homographies
    //            M_blocks{m} = mosaic_calc_size(M_blocks{m},algo_param);

    //            % Fill non empty block indexes
    //            non_empty_blocks_indexes(index) = m;
    //            index = index + 1;

    //        end

    //    end
    //    %hold off
    //    %pause(eps)
    //    multiWaitbar( 'Blocks blending progress', 'Close');

    //    %% Compute gain compensation method (see OpenCV doc as it is the same implementation)
    //    fprintf('Compensate exposure between blocks\n');
    //    block_nb = length ( non_empty_blocks_indexes );
    //    I = zeros(block_nb, block_nb, d);
    //    G = zeros(block_nb, d);
    //    N = zeros(block_nb, block_nb);

    //    for j=1:block_nb

    //        multiWaitbar( 'Compensate exposure between blocks', 'Value', j/block_nb);

    //        for k=j:block_nb

    //            % Search intersection between block regions %%%%%%%%%%%%%%%%%%%%%%%%
    //            m = non_empty_blocks_indexes(j);
    //            n = non_empty_blocks_indexes(k);
    //            if and(~isempty(P_eff_blocks{m}),~isempty(P_eff_blocks{n}))

    //                P_inter = PolygonClip(P_eff_blocks{m},P_eff_blocks{n},1);
    //                if ~isempty(P_inter)

    //                    % Open first image
    //                    Rows_range1 = [min(P_inter.y) max(P_inter.y)-1] - min(P_eff_blocks{m}.y)+1; %+1 is for matlab starting at 1
    //                    Cols_range1 = [min(P_inter.x) max(P_inter.x)-1] - min(P_eff_blocks{m}.x)+1; %
    //                    block_one = imread(fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m)),...
    //                        'PixelRegion',{Rows_range1, Cols_range1});


    //                    % Open second image
    //                    Rows_range2 = [min(P_inter.y) max(P_inter.y)-1] - min(P_eff_blocks{n}.y)+1; %(+1-1)
    //                    Cols_range2 = [min(P_inter.x) max(P_inter.x)-1] - min(P_eff_blocks{n}.x)+1; %(+1-1)
    //                    block_two =imread(fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',n)),...
    //                        'PixelRegion',{Rows_range2, Cols_range2});

    //                    % Common area mask
    //                    common_mask = and( block_one(:,:,end)~=0 , block_two(:,:,end)~=0 );
    //                    N(j,k) = sum(sum(common_mask~=0));
    //                    N(k,j) = N(j,k);

    //                    % Compute mean intensities
    //                    for l=1:d

    //                        if ( N(j,k) > 0 )

    //                            single_ch_block = block_one(:,:,l);
    //                            I(j,k,l) = mean( mean( single_ch_block( common_mask~=0 ) ));
    //                            single_ch_block = block_two(:,:,l);
    //                            I(k,j,l) = mean( mean( single_ch_block( common_mask~=0 ) ));

    //                        else

    //                            I(j,k,l) = 0;
    //                            I(k,j,l) = 0;

    //                        end

    //                    end

    //                    clear block_one block_two Rows_range1 Cols_range1 Rows_range2 Cols_range2

    //                end

    //            end

    //        end

    //    end

    //    multiWaitbar( 'Compensate exposure between blocks', 'Close');

    //    % Invert linear system to estimate the gains
    //    alpha = 0.5;
    //    beta = 100;
    //    for l=1:d
    //        A = zeros(block_nb, block_nb);
    //        b = zeros(block_nb, 1);

    //        for m=1:block_nb
    //            for n=1:block_nb
    //                b(m) = b(m) + beta*N(m,n);
    //                A(m,m) = A(m,m) + beta*N(m,n);
    //                if (m~=n)
    //                    A(m,m) = A(m,m) + 2*alpha*I(m,n,l)*I(m,n,l)*N(m,n);
    //                    A(m,n) = A(m,n) - 2*alpha*I(m,n,l)*I(n,m,l)*N(m,n);
    //                end
    //            end
    //        end

    //        gains = SVD_solve(A,b);
    //        G(:,l) = gains;

    //    end

    //    % Correct images according to their gains and compute colors cum distrib
    //    cdf_mean = zeros(256,d); N_tot = zeros(d,1);
    //    for k=1:block_nb

    //        multiWaitbar( 'Correct block images according to their gain', 'Value', k/block_nb);

    //        m = non_empty_blocks_indexes(k);
    //        if (~isempty(M_blocks_indexes{m}))
    //            block_image = imread(fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m)));
    //            for l=1:d
    //                block_image(:,:,l) = G(k,l)*block_image(:,:,l);
    //                %block_image(:,:,l) = block_image(:,:,l);

    //                % Compute cumulative distribution function if needed for stretching
    //                if (algo_param.cosmetic_stretch==1)

    //                    for l=1:d
    //                        single_ch_block = block_image(:,:,l);
    //                        [cdf_y, cdf_x, n] = cdfcompute(reshape(single_ch_block(block_image(:,:,end)~=0),1,[]));
    //                        [ cdf_y_filled ] = fillwithprevious( cdf_y, cdf_x, 0:255 );

    //                        cdf_mean(:,l) = (N_tot(l)*cdf_mean(:,l) + n*cdf_y_filled')/(N_tot(l)+n);
    //                        N_tot(l) = N_tot(l)+n;
    //                    end

    //                    clear single_ch_block cdf_y cdf_x cdf_y_filled
    //                end

    //            end
    //            write_tiff_alpha(block_image,fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m)));
    //            clear block_image
    //        end

    //    end
    //    multiWaitbar( 'Correct block images according to their gain', 'Close');


    //    %% Strech images if needed
    //    if (algo_param.cosmetic_stretch==1)

    //        for m=1:x_bl_nb*y_bl_nb

    //            multiWaitbar( 'Compensate exposure between blocks', 'Value', j/block_nb);

    //            if (~isempty(M_blocks_indexes{m}))
    //                block_image = imread(fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m)));
    //                for l=1:d
    //                    color_lim = [max(find(cdf_mean(:,l)<0.002))-1, min(find(cdf_mean(:,l)>1-0.002))-1];
    //                    single_ch_block = block_image(:,:,l);
    //                    block_image(:,:,l) = cv.histogram_stretch(single_ch_block,block_image(:,:,end),color_lim,[0 255]);

    //                end
    //                write_tiff_alpha(block_image,fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m)));
    //                clear block_image color_lim single_ch_block
    //            end
    //        end

    //        multiWaitbar( 'Compensate exposure between blocks', 'Close');

    //    end

    //    %% Blend junctions between blocks %%

    //    % First in one direction
    //    for m=1:x_bl_nb

    //        multiWaitbar( 'Merge blocks by columns', 'Value', m/x_bl_nb);

    //        for n=1:y_bl_nb-1
    //            if and(~isempty(P_eff_blocks{m,n}),~isempty(P_eff_blocks{m,n+1}))

    //                % Search intersection between regions %%%%%%%%%%%%%%%%%%%%%%%%
    //                P_inter = PolygonClip(P_eff_blocks{m,n},P_eff_blocks{m,n+1},1);

    //                if ~isempty(P_inter)



    //                    % Add 5% margin for blending %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //                    max_P_inter_x = max(P_inter.x);
    //                    min_P_inter_x = min(P_inter.x);
    //                    max_P_inter_y = max(P_inter.y);
    //                    min_P_inter_y = min(P_inter.y);
    //                    P_inter_x_size = max_P_inter_x-min_P_inter_x;
    //                    P_inter_y_size = max_P_inter_y-min_P_inter_y;

    //                    P_inter_5p = P_inter;
    //                    P_inter_5p.x(P_inter.x == max_P_inter_x ) = ceil(max_P_inter_x+P_inter_x_size*0.05);
    //                    P_inter_5p.x(P_inter.x == min_P_inter_x ) = floor(min_P_inter_x-P_inter_x_size*0.05);
    //                    P_inter_5p.y(P_inter.y == max_P_inter_y ) = ceil(max_P_inter_y+P_inter_y_size*0.05);
    //                    P_inter_5p.y(P_inter.y == min_P_inter_y ) = floor(min_P_inter_y-P_inter_y_size*0.05);

    //                    % Extract the images to be blended %%%%%%%%%%%%%%%%%%%%%%%%%%%
    //                    P_first_block = PolygonClip(P_eff_blocks{m,n},P_inter_5p,1);
    //                    Rows_range1 = [min(P_first_block.y) max(P_first_block.y)-1] - min(P_eff_blocks{m,n}.y)+1; %(+1-1)
    //                    Cols_range1 = [min(P_first_block.x) max(P_first_block.x)-1] - min(P_eff_blocks{m,n}.x)+1; %(+1-1)
    //                    block_images{1} = imread(fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m+(n-1)*x_bl_nb)),...
    //                        'PixelRegion',{Rows_range1, Cols_range1});
    //                    block_masks{1} = block_images{1}(:,:,end);
    //                    block_images{1} = block_images{1}(:,:,1:end-1);
    //                    corners{1} = [min(P_first_block.x) min(P_first_block.y)];

    //                    P_second_block = PolygonClip(P_eff_blocks{m,n+1},P_inter_5p,1);
    //                    Rows_range2 = [min(P_second_block.y) max(P_second_block.y)-1] - min(P_eff_blocks{m,n+1}.y)+1; %(+1-1)
    //                    Cols_range2 = [min(P_second_block.x) max(P_second_block.x)-1] - min(P_eff_blocks{m,n+1}.x)+1; %(+1-1)
    //                    block_images{2} =imread(fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m+(n+1-1)*x_bl_nb)),...
    //                        'PixelRegion',{Rows_range2, Cols_range2});
    //                    block_masks{2} = block_images{2}(:,:,end);
    //                    block_images{2} = block_images{2}(:,:,1:end-1);
    //                    corners{2} = [min(P_second_block.x) min(P_second_block.y)];

    //                    % Blend them %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //                    algo_param.blending_method = 'feather';
    //                    %algo_param.seam_method = 'no';
    //                    algo_param.expo_comp_method = 'no';
    //                    [BlendedEdges, BlendedEdges_mask]= cv.mosaicColorSepBlender(block_images,block_masks,corners,...
    //                        '--try_gpu','no','--seam',algo_param.seam_method,'--expos_comp',algo_param.expo_comp_method,'--blend',algo_param.blending_method);

    //                    clear block_images block_masks

    //                    % Complete and update the block one
    //                    block_one = imread(fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m+(n-1)*x_bl_nb)));
    //                    block_one_mask = block_one(:,:,end);
    //                    block_one = block_one(:,:,1:end-1);

    //                    % Compute indexes to be updated and update
    //                    block_one_shift = corners{1} - min(corners{1}, corners{2});
    //                    block_rows_range = block_one_shift(2)+(1:Rows_range1(2)-Rows_range1(1)+1);
    //                    block_cols_range = block_one_shift(1)+(1:Cols_range1(2)-Cols_range1(1)+1);

    //                    block_extraction = block_one(Rows_range1(1):Rows_range1(2),Cols_range1(1):Cols_range1(2),:);
    //                    block_extraction_mask = block_one_mask(Rows_range1(1):Rows_range1(2),Cols_range1(1):Cols_range1(2));

    //                    blending_extraction = BlendedEdges(block_rows_range,block_cols_range,:);
    //                    blending_extraction_mask = BlendedEdges_mask(block_rows_range,block_cols_range);

    //                    indexes_to_update = blending_extraction_mask ~= 0;
    //                    indexes_to_update(:,:,2)=indexes_to_update; indexes_to_update(:,:,3)=indexes_to_update(:,:,1);
    //                    block_extraction(indexes_to_update) = blending_extraction(indexes_to_update);
    //                    block_extraction_mask = 255*or(block_extraction_mask ~= 0, blending_extraction_mask ~= 0);

    //                    block_one(Rows_range1(1):Rows_range1(2),Cols_range1(1):Cols_range1(2),:) = block_extraction;

    //                    block_one_mask(Rows_range1(1):Rows_range1(2),Cols_range1(1):Cols_range1(2),:) = block_extraction_mask;

    //                    block_one(:,:,size(block_one,3)+1) = block_one_mask;
    //                    write_tiff_alpha(block_one,fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m+(n-1)*x_bl_nb)));

    //                    clear block_one block_one_mask block_extraction blending_extraction_mask blending_extraction blending_extraction_mask

    //                    % Complete and update the block two
    //                    block_two_shift = corners{2} - min(corners{1}, corners{2});
    //                    block_two = imread(fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m+(n+1-1)*x_bl_nb)));
    //                    block_two_mask = block_two(:,:,end);
    //                    block_two = block_two(:,:,1:end-1);


    //                    block_rows_range = block_two_shift(2)+(1:Rows_range2(2)-Rows_range2(1)+1);
    //                    block_cols_range = block_two_shift(1)+(1:Cols_range2(2)-Cols_range2(1)+1);

    //                    block_extraction = block_two(Rows_range2(1):Rows_range2(2),Cols_range2(1):Cols_range2(2),:);
    //                    block_extraction_mask = block_two_mask(Rows_range2(1):Rows_range2(2),Cols_range2(1):Cols_range2(2));

    //                    blending_extraction = BlendedEdges(block_rows_range,block_cols_range,:);
    //                    blending_extraction_mask = BlendedEdges_mask(block_rows_range,block_cols_range);

    //                    indexes_to_update = blending_extraction_mask ~= 0;
    //                    indexes_to_update(:,:,2)=indexes_to_update; indexes_to_update(:,:,3)=indexes_to_update(:,:,1);
    //                    block_extraction(indexes_to_update) = blending_extraction(indexes_to_update);
    //                    block_extraction_mask = 255*or(block_extraction_mask ~= 0, blending_extraction_mask ~= 0);

    //                    block_two(Rows_range2(1):Rows_range2(2),Cols_range2(1):Cols_range2(2),:) = block_extraction;

    //                    block_two_mask(Rows_range2(1):Rows_range2(2),Cols_range2(1):Cols_range2(2),:) = block_extraction_mask;

    //                    block_two(:,:,size(block_two,3)+1) = block_two_mask;
    //                    write_tiff_alpha(block_two,fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m+(n+1-1)*x_bl_nb)));

    //                    clear block_two block_two_mask BlendedEdges BlendedEdges_mask block_extraction blending_extraction_mask blending_extraction blending_extraction_mask

    //                end

    //            end
    //        end
    //    end
    //    multiWaitbar( 'Merge blocks by columns', 'Close');
    //    hold off

    //    % Then in the other direction
    //    ind1 = 0;
    //    ind2 = 0;
    //    block_ind = 1;
    //    for m=1:x_bl_nb-1

    //        multiWaitbar( 'Merge blocks by rows', 'Value', m/(x_bl_nb-1));

    //        P_line1=[];
    //        P_line2=[];
    //        for n=1:y_bl_nb
    //            % Unify the two polygons lines
    //            if ~isempty(P_eff_blocks{m,n})
    //                if ind1==0
    //                    P_line1=P_eff_blocks{m,n};
    //                else
    //                    P_line1=PolygonClip(P_line1,P_eff_blocks{m,n},3);
    //                end
    //                ind1 = ind1+1;
    //            end

    //            if ~isempty(P_eff_blocks{m+1,n})
    //                if ind2==0
    //                    P_line2=P_eff_blocks{m+1,n};
    //                else
    //                    P_line2=PolygonClip(P_line2,P_eff_blocks{m+1,n},3);
    //                end
    //                ind2 = ind2+1;
    //            end
    //        end
    //        ind1 = 0;
    //        ind2 = 0;

    //        % Search intersection between lines
    //        if and(~isempty(P_line1),~isempty(P_line2))
    //            P_line_inter = PolygonClip(P_line1,P_line2,1);
    //        else
    //            P_line_inter = [];
    //        end

    //        % Loop on the number of independant intersections
    //        for p=1:size(P_line_inter,2)

    //            % Extract the images to be blended %%%%%%%%%%%%%%%%%%%%%%%%%%%
    //            block_ind = 1;
    //            for k=m:m+1
    //                for l=1:y_bl_nb
    //                    if and(~isempty(P_line_inter(p)),~isempty(P_eff_blocks{k,l}))
    //                        P_block{block_ind} = PolygonClip(P_eff_blocks{k,l},P_line_inter(p),1);

    //                        if (~isempty(P_block{block_ind}))

    //                            try

    //                                P_block_5p.x = round(mean(P_block{block_ind}.x)+1.05*(P_block{block_ind}.x-mean(P_block{block_ind}.x)));
    //                                P_block_5p.y = round(mean(P_block{block_ind}.y)+1.05*(P_block{block_ind}.y-mean(P_block{block_ind}.y)));
    //                                P_block{block_ind} = PolygonClip(P_eff_blocks{k,l},P_block_5p,1);
    //                                clear P_block_5p
    //                            catch error
    //                                fprintf('error\n');
    //                            end

    //                            if ~isempty(P_block{block_ind})

    //                                Rows_range{block_ind} = [min(P_block{block_ind}.y) max(P_block{block_ind}.y)-1] - min(P_eff_blocks{k,l}.y)+1; %(+1-1)
    //                                Cols_range{block_ind} = [min(P_block{block_ind}.x) max(P_block{block_ind}.x)-1] - min(P_eff_blocks{k,l}.x)+1; %(+1-1)

    //                                block_images{block_ind} = imread(fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',k+(l-1)*x_bl_nb)),...
    //                                    'PixelRegion',{Rows_range{block_ind}, Cols_range{block_ind}});

    //                                block_masks{block_ind} = block_images{block_ind}(:,:,end);
    //                                block_images{block_ind} = block_images{block_ind}(:,:,1:end-1);

    //                                corners{block_ind} = [min(P_block{block_ind}.x) min(P_block{block_ind}.y)];

    //                                if (block_ind ==1)
    //                                    blocks_origin = corners{block_ind};
    //                                else
    //                                    blocks_origin = min(blocks_origin, corners{block_ind});
    //                                end

    //                                block_ind = block_ind +1;
    //                            end
    //                        end
    //                    end
    //                end
    //            end


    //            if (exist('block_images','var'))

    //                % Blend them %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //                algo_param.blending_method = 'feather';
    //                %algo_param.seam_method = 'no';
    //                algo_param.expo_comp_method = 'no';
    //                [BlendedLines, BlendedLines_mask]= cv.mosaicColorSepBlender(block_images,block_masks,corners,...
    //                    '--try_gpu','no','--seam',algo_param.seam_method,'--expos_comp',algo_param.expo_comp_method,'--blend',algo_param.blending_method);

    //                clear block_images block_masks

    //                % Complete and update the blocks
    //                block_ind = 1;
    //                for k=m:m+1
    //                    for l=1:y_bl_nb
    //                        if (~isempty(P_line_inter(p)) && ~isempty(P_eff_blocks{k,l}) && ~isempty(PolygonClip(P_eff_blocks{k,l},P_line_inter(p),1)) )

    //                            block_shift = corners{block_ind} - blocks_origin;
    //                            block = imread(fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',k+(l-1)*x_bl_nb)));
    //                            block_mask = block(:,:,end);
    //                            block = block(:,:,1:end-1);

    //                            % Compute indexes to be updated and update
    //                            block_rows_range = block_shift(2)+(1:Rows_range{block_ind}(2)-Rows_range{block_ind}(1)+1);
    //                            block_cols_range = block_shift(1)+(1:Cols_range{block_ind}(2)-Cols_range{block_ind}(1)+1);

    //                            block_extraction = block(Rows_range{block_ind}(1):Rows_range{block_ind}(2),Cols_range{block_ind}(1):Cols_range{block_ind}(2),:);
    //                            block_extraction_mask = block_mask(Rows_range{block_ind}(1):Rows_range{block_ind}(2),Cols_range{block_ind}(1):Cols_range{block_ind}(2));

    //                            blending_extraction = BlendedLines(block_rows_range,block_cols_range,:);
    //                            blending_extraction_mask = BlendedLines_mask(block_rows_range,block_cols_range);

    //                            indexes_to_update = blending_extraction_mask ~= 0;
    //                            indexes_to_update(:,:,2)=indexes_to_update; indexes_to_update(:,:,3)=indexes_to_update(:,:,1);
    //                            block_extraction(indexes_to_update) = blending_extraction(indexes_to_update);
    //                            block_extraction_mask = 255*or(block_extraction_mask ~= 0, blending_extraction_mask ~= 0);

    //                            block(Rows_range{block_ind}(1):Rows_range{block_ind}(2),Cols_range{block_ind}(1):Cols_range{block_ind}(2),:) = block_extraction;

    //                            block_mask(Rows_range{block_ind}(1):Rows_range{block_ind}(2),Cols_range{block_ind}(1):Cols_range{block_ind}(2),:) = block_extraction_mask;

    //                            block(:,:,size(block,3)+1) = block_mask;
    //                            write_tiff_alpha(block,fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',k+(l-1)*x_bl_nb)));

    //                            % free memory
    //                            clear block block_mask block_extraction blending_extraction_mask blending_extraction blending_extraction_mask
    //                            block_ind = block_ind +1;
    //                        end
    //                    end
    //                end

    //                clear corners BlendedLines BlendedLines_mask;

    //            end

    //        end

    //    end

    //    multiWaitbar( 'Merge blocks by rows', 'Close');


    //    %% Save mosaic to Geotiff
    //    block_ind = 0;
    //    if isfield(algo_param,'output_filename')
    //        output_filename = algo_param.output_filename;
    //    else
    //        output_filename = 'Blended_Mosaic.tif';
    //    end
    //    [path name ext] = fileparts(output_filename);

    //    for m=1:x_bl_nb*y_bl_nb


    //        multiWaitbar( 'Save blocks to disk', 'Value', m/(x_bl_nb*y_bl_nb));

    //        if (~isempty(M_blocks_indexes{m}))

    //            block_ind = block_ind + 1 ;

    //            % Write Geotiff to disk

    //            input_file = fullfile(algo_param.output_dir,sprintf('Temp_%d.tiff',m));
    //            if length(non_empty_blocks_indexes)>1
    //                output_file = fullfile(algo_param.output_dir,[name sprintf('_%d',block_ind) ext]);
    //            else
    //                output_file = fullfile(algo_param.output_dir,[name ext]);
    //            end

    //            if strcmp(algo_param.utm_hemisphere,'S')

    //                write_command = sprintf('gdal_translate -a_srs "+proj=utm +zone=%d +south" -of GTiff -co "INTERLEAVE=PIXEL" -a_ullr %f %f %f %f -mask 4 --config GDAL_TIFF_INTERNAL_MASK YES %s %s',...
    //                    algo_param.utm_zone,M_blocks{m}.init.mosaic_ullr(1),M_blocks{m}.init.mosaic_ullr(2),M_blocks{m}.init.mosaic_ullr(3),M_blocks{m}.init.mosaic_ullr(4),input_file,output_file);

    //            else

    //                write_command = sprintf('gdal_translate -a_srs "+proj=utm +zone=%d" -of GTiff -co "INTERLEAVE=PIXEL" -a_ullr %f %f %f %f -mask 4 --config GDAL_TIFF_INTERNAL_MASK YES %s %s',...
    //                    algo_param.utm_zone,M_blocks{m}.init.mosaic_ullr(1),M_blocks{m}.init.mosaic_ullr(2),M_blocks{m}.init.mosaic_ullr(3),M_blocks{m}.init.mosaic_ullr(4),input_file,output_file);

    //            end


    //            system(write_command);
    //            delete(input_file);

    //        end

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

}
