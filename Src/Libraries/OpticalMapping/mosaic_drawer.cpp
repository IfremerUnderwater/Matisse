#include "mosaic_drawer.h"
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
#include "raster_georeferencer.h"
#include "file_img_exposure_compensate.h"
#include "stdvectoperations.h"
#include "file_utils.h"
#include <math.h>
#include "gdal_translate_wrapper.h"
#include "float.h"
#include "file_image.h"

using namespace std;
using namespace cv;
using namespace cv::detail;
using namespace basic_processing;
using namespace matisse_image;
using namespace nav_tools;
using namespace system_tools;

namespace optical_mapping {

MosaicDrawer::MosaicDrawer(QString _drawing_Options)
{

    // Default command line args
    m_d_options.try_gpu = false;
    m_d_options.seam_megapix = 0.1;
    m_d_options.expos_comp_type = ExposureCompensator::GAIN_BLOCKS;
    m_d_options.gain_block = true;
    m_d_options.seam_find_type = "gc_color";
    m_d_options.blend_type = Blender::MULTI_BAND;
    m_d_options.blend_strength = 1;

    this->parseAndAffectOptions( _drawing_Options );
}




int MosaicDrawer::parseAndAffectOptions(QString _drawing_options)
{

    QStringList argv = _drawing_options.split(" ");
    int argc = argv.size();

    for (int i = 0; i < argc; ++i)
    {
        if (argv[i] == "--try_gpu")
        {
            if (argv[i + 1] == "no")
                m_d_options.try_gpu = false;
            else if (argv[i + 1] == "yes")
                m_d_options.try_gpu = true;
            else
            {
                qDebug()<< "Bad --try_gpu flag value\n";
                return -1;
            }
            i++;
        }
        else if (argv[i] == "--seam_megapix")
        {
            m_d_options.seam_megapix = atof(argv[i + 1].toLocal8Bit().data());
            i++;
        }
        else if (argv[i] == "--expos_comp")
        {
            if (argv[i + 1] == "no"){
                m_d_options.expos_comp_type = ExposureCompensator::NO;
                m_d_options.gain_block = false;
            }
            else if (argv[i + 1] == "gain"){
                m_d_options.expos_comp_type = ExposureCompensator::GAIN;
                m_d_options.gain_block = false;
            }
            else if (argv[i + 1] == "gain_blocks"){
                m_d_options.gain_block = true;
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
                m_d_options.seam_find_type = argv[i + 1];
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
                m_d_options.blend_type = Blender::NO;
            else if (argv[i + 1] == "feather")
                m_d_options.blend_type = Blender::FEATHER;
            else if (argv[i + 1] == "multiband")
                m_d_options.blend_type = Blender::MULTI_BAND;
            else
            {
                qDebug()<< "Bad blending method\n";
                return -1;
            }
            i++;
        }
        else if (argv[i] == "--blend_strength")
        {
            m_d_options.blend_strength = static_cast<float>(atof(argv[i + 1].toLocal8Bit().data()));
            i++;
        }
    }

    return 0;
}

void MosaicDrawer::drawAndBlend(const MosaicDescriptor &_mosaic_d_p, UMat &_mosaic_image_p, UMat &_mosaic_image_mask_p)
{

    std::vector<UMat> images_warped;
    std::vector<UMat> masks_warped;
    std::vector<Point> corners;

    int cam_num = _mosaic_d_p.cameraNodes().size();

    images_warped.resize(cam_num);
    masks_warped.resize(cam_num);
    corners.resize(cam_num);

    int i=0;

    // Project each image on the mosaicking plane
    foreach (ProjectiveCamera* cam, _mosaic_d_p.cameraNodes()) {

        cam->projectImageOnMosaickingPlane(images_warped[i], masks_warped[i], corners[i]);
        cam->image()->releaseImageData();
        i++;
    }

    drawAndBlend(images_warped, masks_warped, corners, _mosaic_image_p, _mosaic_image_mask_p);

}

void MosaicDrawer::drawAndBlend(std::vector<UMat> &_images_warped_p, std::vector<UMat> &_masks_warped_p, std::vector<Point> &_corners_p, UMat &_mosaic_image_p, UMat &_mosaic_image_mask_p)
{

    bool colored_images = true;

    int num_images = static_cast<int>(_images_warped_p.size());
    vector<Size> sizes(num_images);

    if (_images_warped_p[0].channels() == 3){
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
        // opencv331
        vector<UMat> red_images(num_images);
        vector<UMat> green_images(num_images);
        vector<UMat> blue_images(num_images);
        vector<UMat> temp_rgb(3);

        for (int i=0; i<num_images; i++){

            sizes[i] = _images_warped_p[i].size();

            // Split channels
            split(_images_warped_p[i], temp_rgb);
            red_images[i] = temp_rgb[0].clone();
            green_images[i] = temp_rgb[1].clone();
            blue_images[i] = temp_rgb[2].clone();

            mean_col_nb += _images_warped_p[i].cols;
            mean_row_nb += _images_warped_p[i].rows;

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

        if (m_d_options.gain_block){

            Bl_per_image = sqrt(10e9/(100*_images_warped_p.size()*_images_warped_p.size())); // 10e9 is for 1Giga and 100 is due to the number of allocations
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
            red_compensator = ExposureCompensator::createDefault(m_d_options.expos_comp_type);
            green_compensator = ExposureCompensator::createDefault(m_d_options.expos_comp_type);
            blue_compensator = ExposureCompensator::createDefault(m_d_options.expos_comp_type);
        }

        //(Do not remove the following three loops, they are needed to minimize memory use)
        // Process exposure compensation for the red channel

        for (int i = 0; i < num_images; ++i){

            cvtColor(red_images[i], red_images[i], COLOR_GRAY2BGR);

        }
        red_compensator->feed(_corners_p, red_images, _masks_warped_p);
        red_images.clear();

        // Process exposure compensation for the green channel
        for (int i = 0; i < num_images; ++i){

            cvtColor(green_images[i], green_images[i], COLOR_GRAY2BGR);

        }
        green_compensator->feed(_corners_p, green_images, _masks_warped_p);
        green_images.clear();

        // Process exposure compensation for the blue channel
        for (int i = 0; i < num_images; ++i){

            cvtColor(blue_images[i], blue_images[i], COLOR_GRAY2BGR);

        }
        blue_compensator->feed(_corners_p, blue_images, _masks_warped_p);
        blue_images.clear();

        // Compensate exposure
        for (int img_idx = 0; img_idx < num_images; ++img_idx){
            split(_images_warped_p[img_idx], temp_rgb);
            cvtColor(temp_rgb[0], temp_rgb[0], COLOR_GRAY2BGR);
            cvtColor(temp_rgb[1], temp_rgb[1], COLOR_GRAY2BGR);
            cvtColor(temp_rgb[2], temp_rgb[2], COLOR_GRAY2BGR);

            red_compensator->apply(img_idx, _corners_p[img_idx], temp_rgb[0], _masks_warped_p[img_idx]);
            green_compensator->apply(img_idx, _corners_p[img_idx], temp_rgb[1], _masks_warped_p[img_idx]);
            blue_compensator->apply(img_idx, _corners_p[img_idx], temp_rgb[2], _masks_warped_p[img_idx]);

            cvtColor(temp_rgb[0], temp_rgb[0], COLOR_BGR2GRAY);
            cvtColor(temp_rgb[1], temp_rgb[1], COLOR_BGR2GRAY);
            cvtColor(temp_rgb[2], temp_rgb[2], COLOR_BGR2GRAY);

            merge(temp_rgb,_images_warped_p[img_idx]);
        }

    }else{ // Process B&W Images (compensation)

        for (int i=0; i<num_images; i++){

            sizes[i] = _images_warped_p[i].size();
            cvtColor(_images_warped_p[i], _images_warped_p[i], COLOR_GRAY2BGR);

            mean_col_nb += _images_warped_p[i].cols;
            mean_row_nb += _images_warped_p[i].rows;

        }

        mean_col_nb /= num_images;
        mean_row_nb /= num_images;

        // Convert images for seam and exposure processing
        vector<Mat> images_warped_p_f(num_images);

        for (int i = 0; i < num_images; ++i){

            _images_warped_p[i].convertTo(images_warped_p_f[i], CV_32F);

        }

        // Create image compensator
        double Bl_per_image;
        int Bl_num_col,Bl_num_row,Bl_w,Bl_h;
        Ptr<ExposureCompensator> compensator;

        if (m_d_options.gain_block){

            Bl_per_image = sqrt(10e9/(100*_images_warped_p.size()*_images_warped_p.size())); // 10e9 is for 1Giga and 100 is due to the number of allocations
            Bl_num_col = ceil(sqrt(Bl_per_image*(double)mean_row_nb/(double)mean_col_nb));
            Bl_num_row = ceil(Bl_num_col*(double)mean_col_nb/(double)mean_row_nb);

            Bl_w = (mean_col_nb-1)/(Bl_num_col-1);
            Bl_h = (mean_row_nb-1)/(Bl_num_row-1);

            Bl_w = max(Bl_w, 50);
            Bl_h = max(Bl_h, 50);

            qDebug()<< "Blocks Gain Compensator with Bl_w = "<< Bl_w << "Bl_h = "<< Bl_h <<"\n";

            compensator = new BlocksGainCompensator(Bl_w,Bl_h);
        }else{
            compensator = ExposureCompensator::createDefault(m_d_options.expos_comp_type);
        }

        compensator->feed(_corners_p, _images_warped_p, _masks_warped_p);
    }

    // Convert images for seam processing *********************************************************************
    vector<UMat> images_warped_p_f(num_images);
    vector<UMat> masks_warped_p_seam(num_images);
    vector<Point> corners_p_seam(num_images);
    double seam_scale;

    seam_scale = min(1.0, sqrt(m_d_options.seam_megapix * 1e6 / (double)(mean_row_nb*mean_col_nb)));
    qDebug()<< "Theoric seam_scale = " << seam_scale <<"\n";

    for (int i = 0; i < num_images; ++i){

        if (seam_scale < 1){

            // resize image to reduce seam finding computing time
            resize(_images_warped_p[i], images_warped_p_f[i], Size(), seam_scale, seam_scale);
            images_warped_p_f[i].convertTo(images_warped_p_f[i], CV_32F);

            // same for the mask
            resize(_masks_warped_p[i],masks_warped_p_seam[i], Size(), seam_scale, seam_scale);

            // same for the corners_p
            corners_p_seam[i] = seam_scale*_corners_p[i];

        }else{

            _images_warped_p[i].convertTo(images_warped_p_f[i], CV_32F);
        }

    }

    // Run the seam finder ...
    qDebug()<< "Seam finder run..." <<m_d_options.seam_find_type;

    Ptr<SeamFinder> seam_finder;
    if (m_d_options.seam_find_type == "no")
        seam_finder = new detail::NoSeamFinder();
    else if (m_d_options.seam_find_type == "voronoi")
        seam_finder = new detail::VoronoiSeamFinder();
    else if (m_d_options.seam_find_type == "gc_color")
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
    else if (m_d_options.seam_find_type == "gc_colorgrad")
    {
#ifdef HAVE_OPENCV_GPU
        if (dOptions.tryGpu )
            seam_finder = new detail::GraphCutSeamFinderGpu(GraphCutSeamFinderBase::COST_COLOR_GRAD);
        else
#endif
            seam_finder = new detail::GraphCutSeamFinder(GraphCutSeamFinderBase::COST_COLOR_GRAD);
    }
    else if (m_d_options.seam_find_type == "dp_color")
        seam_finder = new detail::DpSeamFinder(DpSeamFinder::COLOR);
    else if (m_d_options.seam_find_type == "dp_colorgrad")
        seam_finder = new detail::DpSeamFinder(DpSeamFinder::COLOR_GRAD);
    if (seam_finder.empty())
    {
        qDebug()<< "Can't create the following seam finder (use the default one instead)\n";
    }

    // Run the seam finder
    if (seam_scale < 1){
        seam_finder->find(images_warped_p_f, corners_p_seam, masks_warped_p_seam);
    }else{
        seam_finder->find(images_warped_p_f, _corners_p, _masks_warped_p);
    }


    // Release unused images
    corners_p_seam.clear();
    images_warped_p_f.clear();

    qDebug()<< "Compositing...\n";

#if ENABLE_LOG
    t = getTickCount();
#endif

    UMat img_warped_s;
    UMat dilated_mask, seam_mask, mask_warped;
    Ptr<Blender> blender;

    // Initialize the blender
    blender = Blender::createDefault(m_d_options.blend_type, m_d_options.try_gpu);
    Size dst_sz = resultRoi(_corners_p, sizes).size();
    float blend_width = sqrt(static_cast<float>(dst_sz.area())) * m_d_options.blend_strength / 100.f;
    if (blend_width < 1.f)
        blender = Blender::createDefault(Blender::NO, m_d_options.try_gpu);
    else if (m_d_options.blend_type == Blender::MULTI_BAND)
    {
        MultiBandBlender* mb = dynamic_cast<MultiBandBlender*>(static_cast<Blender*>(blender));
        mb->setNumBands(min(static_cast<int>(ceil(log(blend_width)/log(2.)) - 1.),(int)4));
        qDebug()<<  "Multi-band blender, number of bands: " << mb->numBands();
    }
    else if (m_d_options.blend_type == Blender::FEATHER)
    {
        FeatherBlender* fb = dynamic_cast<FeatherBlender*>(static_cast<Blender*>(blender));
        fb->setSharpness(1.f/blend_width);
        qDebug()<< "Feather blender, sharpness: "<< fb->sharpness();
    }
    blender->prepare(_corners_p, sizes);

    for (int img_idx = 0; img_idx < num_images; ++img_idx)
    {

        _images_warped_p[img_idx].convertTo(img_warped_s, CV_16S);

        if (seam_scale < 1){
            // Upscale the seaming mask
            dilate(masks_warped_p_seam[img_idx], dilated_mask, Mat());
            //dilate(masksWarped_p[img_idx], dilated_mask, Mat());
            resize(dilated_mask, seam_mask, _masks_warped_p[img_idx].size());
            Mat mmask_warped = seam_mask.getMat(ACCESS_READ) & _masks_warped_p[img_idx].getMat(ACCESS_READ);
            mask_warped = mmask_warped.getUMat(ACCESS_READ);

            // Blend the current image
            blender->feed(img_warped_s, mask_warped, _corners_p[img_idx]);
        }else{
            blender->feed(img_warped_s, _masks_warped_p[img_idx], _corners_p[img_idx]);
        }


    }

    qDebug()<< "Blend...";

    blender->blend(_mosaic_image_p, _mosaic_image_mask_p);

#if ENABLE_LOG
    LOGLN("Compositing, time: " << ((getTickCount() - t) / getTickFrequency()) << " sec");

    LOGLN("Finished, total time: " << ((getTickCount() - app_start_time) / getTickFrequency()) << " sec");
#endif
    // Convert result to 8U
    _mosaic_image_p.convertTo(_mosaic_image_p,CV_8U);

}

QStringList MosaicDrawer::blockDrawBlendAndWrite(const MosaicDescriptor &_mosaic_d_p, Point2d block_size_p, QString writing_path_p, QString _prefix_p)
{

    int x_block_number, y_block_number, x_overlap_size, y_overlap_size;
    int x_last_column_size, y_last_row_size;

    QStringList output_files;
    //bool gainCompRequired = ( dOptions.exposCompType != ExposureCompensator::NO );

    // Create tmp folder to contain temporary files
    QDir temp_dir(writing_path_p + QDir::separator() + QString("tmp"));
    if (!temp_dir.exists()) {
        temp_dir.mkpath(".");
    }

    // We first backup drawing options as we will change them on blocks blending iterations
    drawingOptions d_options_backup = m_d_options;

    // % overlap hard coded for the moment
    x_overlap_size = (int)(0.20*block_size_p.x);
    y_overlap_size = (int)(0.20*block_size_p.y);

    // Get complete mosaic size
    Point2d mosaic_size = _mosaic_d_p.mosaicSize();

    // Compute last block size
    x_last_column_size = (int)(mosaic_size.x) % ((int)block_size_p.x-x_overlap_size);
    y_last_row_size = (int)(mosaic_size.y) % ((int)block_size_p.y-y_overlap_size);

    // Deduce number of blocks
    if (x_last_column_size == 0){
        x_block_number = mosaic_size.x / ((int)block_size_p.x-x_overlap_size);
    }else{
        x_block_number = (mosaic_size.x - x_last_column_size) / ((int)block_size_p.x-x_overlap_size);
        x_block_number++;
    }

    if (y_last_row_size == 0){
        y_block_number = mosaic_size.y / ((int)block_size_p.y-y_overlap_size);
    }else{
        y_block_number = (mosaic_size.y - y_last_row_size) / ((int)block_size_p.y-y_overlap_size);
        y_block_number++;
    }

    // Allocate and build polygons associated with images
    vector<Polygon*> vp_images_poly;


    for (int k=0; k < _mosaic_d_p.cameraNodes().size(); k++){
        std::vector<double> x,y;
        int x_begin, y_begin, x_end, y_end;
        cv::Point img_corner;
        cv::Size img_size;

        _mosaic_d_p.cameraNodes().at(k)->computeImageExtent(img_corner, img_size);

        // Compute extents
        x_begin = img_corner.x;
        y_begin = img_corner.y;

        x_end = img_corner.x + img_size.width-1;
        y_end = img_corner.y + img_size.height-1;

        // Construct currentPolygon
        Polygon *current_polygon = new Polygon();
        x.push_back(x_begin); x.push_back(x_end); x.push_back(x_end); x.push_back(x_begin);
        y.push_back(y_begin); y.push_back(y_begin); y.push_back(y_end); y.push_back(y_end);
        current_polygon->addContour(x,y);
        x.clear(); y.clear();
        vp_images_poly.push_back(current_polygon);

    }

    // Construct polygons corresponding to blocks areas
    vector<Polygon*> vp_blocks_poly;
    vector<Polygon*> vp_eff_blocks_poly;
    vector<vector<int>*> vv_blocks_img_indexes;

    for (int j=0; j<x_block_number; j++){
        for (int i=0; i<y_block_number; i++){

            Polygon *current_block_polygon = new Polygon();
            Polygon *current_eff_block_polygon = new Polygon();
            vector<int> *current_img_indexes = new vector<int>;
            std::vector<double> x,y;
            int x_begin, y_begin, x_end, y_end;
            int x_current_block_size, y_current_block_size;

            // Handle last block size
            if (j == x_block_number-1){
                x_current_block_size = x_last_column_size;
            }else{
                x_current_block_size = block_size_p.x;
            }

            if (i == y_block_number-1){
                y_current_block_size = y_last_row_size;
            }else{
                y_current_block_size = block_size_p.y;
            }

            // Compute extents
            x_begin = (block_size_p.x-x_overlap_size)*j;
            y_begin = (block_size_p.y-y_overlap_size)*i;

            x_end = x_begin + x_current_block_size-1;
            y_end = y_begin + y_current_block_size-1;

            // Construct currentPolygon
            x.push_back(x_begin); x.push_back(x_end); x.push_back(x_end); x.push_back(x_begin);
            y.push_back(y_begin); y.push_back(y_begin); y.push_back(y_end); y.push_back(y_end);
            current_block_polygon->addContour(x,y);
            x.clear(); y.clear();

            // Find which images intersect with this block
            // and compute effective block (ie. remove non occupied space)

            double img_tlx, img_tly, img_brx, img_bry;
            double block_tlx, block_tly, block_brx, block_bry;

            block_tlx=DBL_MAX;
            block_tly=DBL_MAX;
            block_brx=-DBL_MAX;
            block_bry=-DBL_MAX;

            for (int k=0; k < _mosaic_d_p.cameraNodes().size(); k++){

                Polygon *img_block_intersection = new Polygon;
                vp_images_poly[k]->clip(*current_block_polygon,*img_block_intersection,basic_processing::INT);

                if (!img_block_intersection->isEmpty()){

                    current_img_indexes->push_back(k);
                    img_block_intersection->getBoundingBox(img_tlx, img_tly, img_brx, img_bry);

                    if (img_tlx < block_tlx)
                        block_tlx = img_tlx;

                    if (img_tly < block_tly)
                        block_tly = img_tly;

                    if (img_brx > block_brx)
                        block_brx = img_brx;

                    if (img_bry > block_bry)
                        block_bry = img_bry;

                }

                delete img_block_intersection;
            }

            if (current_img_indexes->size() > 0){
                vp_blocks_poly.push_back(current_block_polygon);
                vv_blocks_img_indexes.push_back(current_img_indexes);

                // Construct currentEffPolygon
                x.push_back(block_tlx); x.push_back(block_brx); x.push_back(block_brx); x.push_back(block_tlx);
                y.push_back(block_tly); y.push_back(block_tly); y.push_back(block_bry); y.push_back(block_bry);
                current_eff_block_polygon->addContour(x,y);
                x.clear(); y.clear();
                vp_eff_blocks_poly.push_back(current_eff_block_polygon);
            }
            else{
                delete current_img_indexes;
                delete current_block_polygon;
                delete current_eff_block_polygon;
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
    for (unsigned int k=0; k < vp_eff_blocks_poly.size(); k++){
        std::vector<UMat> images_warped;
        std::vector<UMat> masks_warped;
        std::vector<Point> corners;
        UMat mosaic_image, mosaic_image_mask;

        int cam_num = vv_blocks_img_indexes[k]->size();

        images_warped.resize(cam_num);
        masks_warped.resize(cam_num);
        corners.resize(cam_num);

        // Project each image on the mosaicking plane (TODO : make it generic for orthophoto 3D/2D mosaicking)
        for (int l=0; l < cam_num; l++) {

            Polygon *img_block_inter = new Polygon();
            vp_images_poly[vv_blocks_img_indexes[k]->at(l)]->clip(*(vp_eff_blocks_poly[k]),*img_block_inter,basic_processing::INT);

            ProjectiveCamera* cam = _mosaic_d_p.cameraNodes().at(vv_blocks_img_indexes[k]->at(l));

            if (*(vp_images_poly[vv_blocks_img_indexes[k]->at(l)]) == *img_block_inter){
                cam->projectImageOnMosaickingPlane(images_warped[l], masks_warped[l], corners[l]);

            }else{
                UMat temp_img_warped, temp_mask_warped;
                cam->projectImageOnMosaickingPlane(temp_img_warped, temp_mask_warped, corners[l]);

                // Get useful image part
                double tl_x,tl_y,br_x,br_y;
                img_block_inter->getBoundingBox(tl_x,tl_y,br_x,br_y);

                // Crop images and masks with the bounding box
                temp_img_warped(cv::Rect(tl_x-corners[l].x,tl_y-corners[l].y,br_x-tl_x+1,br_y-tl_y+1)).copyTo(images_warped[l]);
                temp_mask_warped(cv::Rect(tl_x-corners[l].x,tl_y-corners[l].y,br_x-tl_x+1,br_y-tl_y+1)).copyTo(masks_warped[l]);
                corners[l].x = tl_x;
                corners[l].y = tl_y;
            }

            cam->image()->releaseImageData();
            delete img_block_inter;

        }

        // Draw block
        drawAndBlend(images_warped, masks_warped, corners, mosaic_image, mosaic_image_mask);
        QString mosaic_file_path = writing_path_p + QDir::separator() + QString("tmp")
                + QDir::separator() + _prefix_p + QString("_temp%1.tiff").arg(k, 4, 'g', -1, '0');
        imwrite(mosaic_file_path.toStdString().c_str(), mosaic_image);

        QString mosaic_mask_file_path = writing_path_p + QDir::separator() + QString("tmp")
                + QDir::separator() + _prefix_p + QString("_masktemp%1.tiff").arg(k, 4, 'g', -1, '0');
        imwrite(mosaic_mask_file_path.toStdString().c_str(), mosaic_image_mask);

    }


    // Blend junction between pairs of blocks
    vector<Polygon*> vp_blocks_pair_intersect_poly;

    // Adapt settings to junction blending
    m_d_options.seam_megapix = 0.1;
    m_d_options.expos_comp_type = ExposureCompensator::NO;
    m_d_options.gain_block = false;
    m_d_options.seam_find_type = "gc_color";
    m_d_options.blend_type = Blender::FEATHER;
    m_d_options.blend_strength = 1;

    for (unsigned int k=0; k < vp_eff_blocks_poly.size()-1; k++){

        std::vector<UMat> blocks_to_be_blended;
        std::vector<UMat> blocks_to_be_blended_masks;
        std::vector<Point> corners;
        UMat blended_blocks_img, blended_blocks_img_mask;

        blocks_to_be_blended.resize(2);
        blocks_to_be_blended_masks.resize(2);
        corners.resize(2);

        QString img_file_path1, img_file_path2;
        QString mosaic_mask_file_path1, mosaic_mask_file_path2;

        for (unsigned int l=k+1; l<vp_eff_blocks_poly.size(); l++){

            Polygon *blocks_pair_inter = new Polygon();
            vp_eff_blocks_poly[k]->clip(*(vp_eff_blocks_poly[l]),*blocks_pair_inter,basic_processing::INT);
            double tl_x1,tl_y1,br_x1,br_y1;
            double tl_x2,tl_y2,br_x2,br_y2;

            if( !(blocks_pair_inter->isEmpty()) ){

                // Open first block and mask & get corner
                img_file_path1 = writing_path_p + QDir::separator() + QString("tmp") + QDir::separator() + _prefix_p + QString("_temp%1.tiff").arg(k, 4, 'g', -1, '0');
                blocks_to_be_blended[0] = imread(img_file_path1.toStdString().c_str(),cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION).getUMat(ACCESS_READ);

                mosaic_mask_file_path1 = writing_path_p + QDir::separator() + QString("tmp") + QDir::separator() + _prefix_p + QString("_masktemp%1.tiff").arg(k, 4, 'g', -1, '0');
                blocks_to_be_blended_masks[0] = imread(mosaic_mask_file_path1.toStdString().c_str(),IMREAD_GRAYSCALE | cv::IMREAD_IGNORE_ORIENTATION).getUMat(ACCESS_READ);

                vp_eff_blocks_poly[k]->getBoundingBox(tl_x1,tl_y1,br_x1,br_y1);
                corners[0].x = (int) tl_x1;
                corners[0].y = (int) tl_y1;

                // Open second block and mask
                img_file_path2 = writing_path_p + QDir::separator() + QString("tmp") + QDir::separator() + _prefix_p + QString("_temp%1.tiff").arg(l, 4, 'g', -1, '0');
                blocks_to_be_blended[1] = imread(img_file_path2.toStdString().c_str(),cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION).getUMat(ACCESS_READ);

                mosaic_mask_file_path2 = writing_path_p + QDir::separator() + QString("tmp") + QDir::separator() + _prefix_p + QString("_masktemp%1.tiff").arg(l, 4, 'g', -1, '0');
                blocks_to_be_blended_masks[1] = imread(mosaic_mask_file_path2.toStdString().c_str(),cv::IMREAD_GRAYSCALE | cv::IMREAD_IGNORE_ORIENTATION).getUMat(ACCESS_READ);

                vp_eff_blocks_poly[l]->getBoundingBox(tl_x2,tl_y2,br_x2,br_y2);
                corners[1].x = (int) tl_x2;
                corners[1].y = (int) tl_y2;

                // Blend blocks
                drawAndBlend(blocks_to_be_blended, blocks_to_be_blended_masks, corners, blended_blocks_img, blended_blocks_img_mask);

                // Extract and write block 1
                int roi_x1,roi_x2,roi_y1,roi_y2;

                roi_x1 = tl_x1 - std::min(tl_x1, tl_x2);
                roi_x2 = br_x1 - std::min(tl_x1, tl_x2);
                roi_y1 = tl_y1 - std::min(tl_y1, tl_y2);
                roi_y2 = br_y1 - std::min(tl_y1, tl_y2);

                imwrite(img_file_path1.toStdString().c_str(),blended_blocks_img(cv::Rect(roi_x1,roi_y1,roi_x2-roi_x1+1,roi_y2-roi_y1+1)));
                imwrite(mosaic_mask_file_path1.toStdString().c_str(),blended_blocks_img_mask(cv::Rect(roi_x1,roi_y1,roi_x2-roi_x1+1,roi_y2-roi_y1+1)));

                // Extract and write block 2

                roi_x1 = tl_x2 - std::min(tl_x1, tl_x2);
                roi_x2 = br_x2 - std::min(tl_x1, tl_x2);
                roi_y1 = tl_y2 - std::min(tl_y1, tl_y2);
                roi_y2 = br_y2 - std::min(tl_y1, tl_y2);

                imwrite(img_file_path2.toStdString().c_str(),blended_blocks_img(cv::Rect(roi_x1,roi_y1,roi_x2-roi_x1+1,roi_y2-roi_y1+1)));
                imwrite(mosaic_mask_file_path2.toStdString().c_str(),blended_blocks_img_mask(cv::Rect(roi_x1,roi_y1,roi_x2-roi_x1+1,roi_y2-roi_y1+1)));

                vp_blocks_pair_intersect_poly.push_back(blocks_pair_inter);


            }else{
                delete blocks_pair_inter;
            }



        }


    }


    // Find intersections between junctions
    Polygon temp_poly1, temp_poly2, junction_inter_poly;

    if (vp_blocks_pair_intersect_poly.size()!=0){

        for (unsigned int i=0; i<vp_blocks_pair_intersect_poly.size()-1; i++){

            for (unsigned int j=i+1; j<vp_blocks_pair_intersect_poly.size(); j++){

                // Intersect junctions
                vp_blocks_pair_intersect_poly[i]->clip(*(vp_blocks_pair_intersect_poly[j]),temp_poly1,basic_processing::INT);

                // Add intersection
                junction_inter_poly.clip(temp_poly1,temp_poly2,basic_processing::UNION);

                // Affect
                junction_inter_poly = temp_poly2;

            }

        }

    }

    // Blend intersections between junctions
    if (!junction_inter_poly.isEmpty()){

        QString img_block_file_path;
        QString img_block_mask_file_path;

        for(unsigned int c=0; c<junction_inter_poly.contours().size(); c++){

            Polygon *current_junction = new Polygon;
            double cc_x,cc_y, ncc_x, ncc_y;
            vector<double> x, y;

            junction_inter_poly.getContourCenter(cc_x, cc_y, c);

            // Get contour coords
            x = junction_inter_poly.contours()[c].x;
            y = junction_inter_poly.contours()[c].y;

            // Multiply coords to magnify poly
            doubleVectorScalarMult(x, 1.05);
            doubleVectorScalarMult(y, 1.05);

            // Compute new center
            ncc_x = doubleVectorMean(x);
            ncc_y = doubleVectorMean(y);

            // Recenter polygon and round the coord to have integers
            for (unsigned int i=0; i<x.size(); i++){
                x[i] = (int)( x[i] - ncc_x + cc_x );
                y[i] = (int)( y[i] - ncc_y + cc_y );
            }

            // Fill polygon with this contour
            current_junction->addContour(x, y);
            current_junction->updateGpcPolygon();


            // Fill blocks to be blended *****************************************
            std::vector<UMat> blocks_to_be_blended;
            std::vector<UMat> blocks_to_be_blended_masks;
            std::vector<Point> tl_corners, br_corners, tl_blocks_corners;
            std::vector<int> blocks_to_be_blended_indexes;
            UMat blended_blocks_img, blended_blocks_img_mask;

            double tl_x_min,tl_y_min;

            for (unsigned int l=0; l<vp_eff_blocks_poly.size(); l++){

                Polygon *block_junction_inter = new Polygon();
                current_junction->clip(*(vp_eff_blocks_poly[l]),*block_junction_inter,basic_processing::INT);
                double tl_x,tl_y,br_x,br_y;
                double tl_block_x, tl_block_y, br_block_x, br_block_y;

                if( !(block_junction_inter->isEmpty()) ){


                    // Open block and mask & get corner
                    UMat img_temp;

                    vp_eff_blocks_poly[l]->getBoundingBox(tl_block_x, tl_block_y, br_block_x, br_block_y);
                    block_junction_inter->getBoundingBox( tl_x,tl_y,br_x,br_y );

                    img_block_file_path = writing_path_p + QDir::separator() + QString("tmp") + QDir::separator() + _prefix_p + QString("_temp%1.tiff").arg(l, 4, 'g', -1, '0');
                    img_temp = imread(img_block_file_path.toStdString().c_str(),cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION).getUMat(ACCESS_READ);
                    blocks_to_be_blended.push_back( img_temp(Rect(tl_x - tl_block_x, tl_y - tl_block_y, br_x-tl_x+1, br_y-tl_y+1)) );

                    img_block_mask_file_path = writing_path_p + QDir::separator() + QString("tmp") + QDir::separator() + _prefix_p + QString("_masktemp%1.tiff").arg(l, 4, 'g', -1, '0');
                    img_temp = imread(img_block_mask_file_path.toStdString().c_str(),cv::IMREAD_GRAYSCALE | cv::IMREAD_IGNORE_ORIENTATION).getUMat(ACCESS_READ);
                    blocks_to_be_blended_masks.push_back( img_temp(Rect(tl_x - tl_block_x, tl_y - tl_block_y, br_x-tl_x+1, br_y-tl_y+1))  );

                    // Store corners values
                    tl_blocks_corners.push_back( Point( (int) tl_block_x, (int) tl_block_y ) );
                    tl_corners.push_back( Point( (int) tl_x, (int) tl_y ) );
                    br_corners.push_back( Point( (int) br_x, (int) br_y ) );

                    blocks_to_be_blended_indexes.push_back(l);

                    if (blocks_to_be_blended_indexes.size()==1){
                        tl_x_min = tl_x;
                        tl_y_min = tl_y;
                    }else{
                        tl_x_min = std::min(tl_x_min, tl_x);
                        tl_y_min = std::min(tl_y_min, tl_y);
                    }


                }else{
                    delete block_junction_inter;
                }

            }


            // Blend blocks
            drawAndBlend(blocks_to_be_blended, blocks_to_be_blended_masks, tl_corners, blended_blocks_img, blended_blocks_img_mask);

            for ( unsigned int i=0; i<blocks_to_be_blended_indexes.size(); i++ ){

                int l = blocks_to_be_blended_indexes[i];
                img_block_file_path = writing_path_p + QDir::separator() + QString("tmp") + QDir::separator() + _prefix_p + QString("_temp%1.tiff").arg(l, 4, 'g', -1, '0');
                img_block_mask_file_path = writing_path_p + QDir::separator() + QString("tmp") + QDir::separator() + _prefix_p + QString("_masktemp%1.tiff").arg(l, 4, 'g', -1, '0');

                UMat block_img = imread(img_block_file_path.toStdString().c_str(),cv::IMREAD_COLOR|cv::IMREAD_IGNORE_ORIENTATION).getUMat(ACCESS_READ);
                UMat block_img_mask = imread(img_block_mask_file_path.toStdString().c_str(),cv::IMREAD_GRAYSCALE|cv::IMREAD_IGNORE_ORIENTATION).getUMat(ACCESS_READ);


                // Part of the matrix we are interested in
                UMat block_img_roi(block_img, Rect(tl_corners[i].x-tl_blocks_corners[i].x,tl_corners[i].y-tl_blocks_corners[i].y,
                                               br_corners[i].x-tl_corners[i].x+1,br_corners[i].y-tl_corners[i].y+1));
                // This submatrix will be a REFERENCE to PART of full matrix, NOT a copy
                UMat block_img_mask_roi(block_img_mask, Rect(tl_corners[i].x-tl_blocks_corners[i].x,tl_corners[i].y-tl_blocks_corners[i].y,
                                                       br_corners[i].x-tl_corners[i].x+1,br_corners[i].y-tl_corners[i].y+1));

                block_img_roi = blended_blocks_img( Rect(tl_corners[i].x-tl_x_min, tl_corners[i].y-tl_y_min, br_corners[i].x-tl_corners[i].x+1,br_corners[i].y-tl_corners[i].y+1 ) );
                block_img_mask_roi = blended_blocks_img_mask( Rect(tl_corners[i].x-tl_x_min, tl_corners[i].y-tl_y_min, br_corners[i].x-tl_corners[i].x+1,br_corners[i].y-tl_corners[i].y+1 ) );

                // Save image
                imwrite(img_block_file_path.toStdString().c_str(),block_img);
                imwrite(img_block_mask_file_path.toStdString().c_str(),block_img_mask);

            }


            blocks_to_be_blended.clear();
            blocks_to_be_blended_masks.clear();
            tl_corners.clear();
            br_corners.clear();
            tl_blocks_corners.clear();
            blocks_to_be_blended_indexes.clear();

            x.clear();
            y.clear();
            delete current_junction;

        }

    }

    // Write all geotiff files from temp files
    for (unsigned int k=0; k<vp_eff_blocks_poly.size(); k++){

        QString utm_proj_param, utm_hemisphere_option, utm_zone_string;

        // Construct utm proj param options
        utm_zone_string = QString("%1 ").arg(_mosaic_d_p.utmZone())+ _mosaic_d_p.utmHemisphere();
        QStringList utm_params = utm_zone_string.split(" ");

        if ( utm_params.at(1) == "S" ){
            utm_hemisphere_option = QString(" +south");
        }else{
            utm_hemisphere_option = QString("");
        }
        utm_proj_param = QString("+proj=utm +zone=") + utm_params.at(0);

        // Get block corners in pixels
        double block_tl_x, block_tl_y, block_br_x, block_br_y;
        vp_eff_blocks_poly[k]->getBoundingBox(block_tl_x,block_tl_y,block_br_x,block_br_y);


        double block_utm_tl_x = _mosaic_d_p.mosaicOrigin().x + block_tl_x*_mosaic_d_p.pixelSize().x;
        double block_utm_tl_y = _mosaic_d_p.mosaicOrigin().y - block_tl_y*_mosaic_d_p.pixelSize().y;

        double block_utm_br_x = _mosaic_d_p.mosaicOrigin().x + block_br_x*_mosaic_d_p.pixelSize().x;
        double block_utm_br_y = _mosaic_d_p.mosaicOrigin().y - block_br_y*_mosaic_d_p.pixelSize().y;

        QString gdal_options =  QString("-a_srs \"")+ utm_proj_param + QString("\" -of GTiff -co \"COMPRESS=JPEG\" -co \"INTERLEAVE=PIXEL\" -a_ullr %1 %2 %3 %4")
                .arg(block_utm_tl_x,0,'f',2)
                .arg(block_utm_tl_y,0,'f',2)
                .arg(block_utm_br_x,0,'f',2)
                .arg(block_utm_br_y,0,'f',2);

        output_files << _prefix_p + QString("_%1.tiff").arg(k, 4, 'g', -1, '0');
        QString geo_ref_block_img_file_path = writing_path_p + QDir::separator() + output_files.at(output_files.size()-1);

        // Read files
        QString block_img_file_path = writing_path_p + QDir::separator() + QString("tmp") + QDir::separator() + _prefix_p + QString("_temp%1.tiff").arg(k, 4, 'g', -1, '0');
        Mat block_img = imread(block_img_file_path.toStdString().c_str(),cv::IMREAD_COLOR|cv::IMREAD_IGNORE_ORIENTATION);

        QString block_img_mask_file_path = writing_path_p + QDir::separator() + QString("tmp") + QDir::separator() + _prefix_p + QString("_masktemp%1.tiff").arg(k, 4, 'g', -1, '0');
        Mat block_img_mask = imread(block_img_mask_file_path.toStdString().c_str(),cv::IMREAD_GRAYSCALE | cv::IMREAD_IGNORE_ORIENTATION);

        std::vector<cv::Mat> block_img_channels;
        cv::split(block_img, block_img_channels);

        // create alpha channel
        block_img_channels.push_back(block_img_mask);

        // create tempBlock
        QString temp_block_img_file_path = writing_path_p + QDir::separator() + QString("tmp") + QDir::separator() + QString("Temp.png");
        cv::Mat_<cv::Vec4b> dst;
        cv::merge(block_img_channels, dst);
        cv::imwrite(temp_block_img_file_path.toStdString(),dst);

        GdalTranslateWrapper gdal_translate;
        gdal_translate.geoReferenceFile(temp_block_img_file_path,geo_ref_block_img_file_path,gdal_options);

        //RasterGeoreferencer rasterGeoref;
        //rasterGeoref.WriteGeoFile(blockImg, blockImgMask, geoRefBlockImgFilePath,gdalOptions);

    }

    // Restore drawing options
    m_d_options = d_options_backup;

    // Free memory
    for (unsigned int i=0; i<vp_blocks_poly.size(); i++){
        delete vp_blocks_poly.at(i);
    }

    for (unsigned int i=0; i<vp_eff_blocks_poly.size(); i++){
        delete vp_eff_blocks_poly.at(i);
    }

    for (unsigned int i=0; i<vp_images_poly.size(); i++){
        delete vp_images_poly.at(i);
    }

    for (unsigned int i=0; i<vp_blocks_pair_intersect_poly.size(); i++){
        delete vp_blocks_pair_intersect_poly.at(i);
    }

    FileUtils::removeDir(temp_dir.absolutePath());

    return output_files;

}

QStringList MosaicDrawer::writeImagesAsGeoTiff(const MosaicDescriptor& _mosaic_d_p, QString _writing_path_p, QString _prefix_p)
{
    // This method is mainly used in case were navigation is not perfect, so we just want a rough representation of the footprint
    // hence in order to speed up computation we neglect perspective transform (only scaling and rotation will be applied by geotiff info)

    QStringList output_files;

    QString utm_proj_param, utm_hemisphere_option, utm_zone_string;

    // Construct utm proj param options
    utm_zone_string = QString("%1 ").arg(_mosaic_d_p.utmZone()) + _mosaic_d_p.utmHemisphere();
    QStringList utm_params = utm_zone_string.split(" ");

    if (utm_params.at(1) == "S") {
        utm_hemisphere_option = QString(" +south");
    }
    else {
        utm_hemisphere_option = QString("");
    }
    utm_proj_param = QString("+proj=utm +zone=") + utm_params.at(0);

    // Loop on all images
    for (int k = 0; k < _mosaic_d_p.cameraNodes().size(); k++) {
        std::vector<double> x, y;

        // Compute image footprint
        _mosaic_d_p.cameraNodes().at(k)->computeImageFootPrint(x, y);

        Polygon* current_polygon = new Polygon();
        current_polygon->addContour(x, y);

        // Get block corners in pixels
        double TL_x, TL_y, BR_x, BR_y;
        current_polygon->getBoundingBox(TL_x, TL_y, BR_x, BR_y);

        delete current_polygon;
        x.clear(); y.clear();

        UMat img_warped;
        UMat img_warped_mask;
        cv::Point corner;

        _mosaic_d_p.cameraNodes().at(k)->projectImageOnMosaickingPlane(img_warped, img_warped_mask, corner);

        double utm_tl_x = _mosaic_d_p.mosaicOrigin().x + TL_x * _mosaic_d_p.pixelSize().x;
        double utm_tl_y = _mosaic_d_p.mosaicOrigin().y - TL_y * _mosaic_d_p.pixelSize().y;

        double utm_br_x = _mosaic_d_p.mosaicOrigin().x + BR_x * _mosaic_d_p.pixelSize().x;
        double utm_br_y = _mosaic_d_p.mosaicOrigin().y - BR_y * _mosaic_d_p.pixelSize().y;

        QString gdal_options = QString("-a_srs \"") + utm_proj_param + QString("\" -of GTiff -co \"COMPRESS=JPEG\" -co \"INTERLEAVE=PIXEL\" -a_ullr %1 %2 %3 %4")
            .arg(utm_tl_x, 0, 'f', 2)
            .arg(utm_tl_y, 0, 'f', 2)
            .arg(utm_br_x, 0, 'f', 2)
            .arg(utm_br_y, 0, 'f', 2);

        output_files << _prefix_p + QString("_%1.tiff").arg(k, 4, 'g', -1, '0');
        QString geo_ref_img_file_path = _writing_path_p + QDir::separator() + output_files.at(output_files.size() - 1);

        QString temp_input = _writing_path_p + QDir::separator() + _prefix_p + QString("_%1.png").arg(k, 4, 'g', -1, '0');

        //FileImage* input_image =dynamic_cast<MatisseCommon::FileImage*>(mosaicD_p.cameraNodes().at(k)->image());

        std::vector<UMat> img_warped_channels;
        img_warped_channels.push_back(img_warped);
        img_warped_channels.push_back(img_warped_mask);
        cv::Mat_<cv::Vec4b> img_warped_with_alpha;
        cv::merge(img_warped_channels, img_warped_with_alpha);
        cv::imwrite(temp_input.toStdString(), img_warped_with_alpha);

        GdalTranslateWrapper gdal_translate;
        gdal_translate.geoReferenceFile(temp_input, geo_ref_img_file_path, gdal_options);

        // remove temp file
        QFile file(temp_input);
        file.remove();

    }

    return output_files;
}

QStringList MosaicDrawer::outputMosaicImagesAsIs(const MosaicDescriptor& _mosaic_d_p, QString _writing_path_p, QString _prefix_p)
{
    Q_UNUSED(_prefix_p)
    QStringList output_files;

    // Loop on all images
    for (int k = 0; k < _mosaic_d_p.cameraNodes().size(); k++) {

        FileImage* input_image = dynamic_cast<matisse_image::FileImage*>(_mosaic_d_p.cameraNodes().at(k)->image());

        if (input_image != nullptr)
        {
            QString out_string = _writing_path_p + QDir::separator() + input_image->getFileName();
            cv::imwrite(out_string.toStdString(), *(input_image->imageData()));
        }

    }

    return output_files;
}

} // namespace optical_mapping
