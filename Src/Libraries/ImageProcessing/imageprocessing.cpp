#include "imageprocessing.h"
#include "stdvectoperations.h"

using namespace cv;

void IMAGEPROCESSINGSHARED_EXPORT histogramStretch(Mat &_in_img, Mat &_in_mask, Point _low_high_in, Point _low_high_out,  Mat &_stretched_img)
{


    // Init
    _stretched_img = Mat::zeros( _in_img.size(), _in_img.type() );
    int ch_num = _in_img.channels();
    bool process_pixel=false;

    /// Stretches the image from low_high_in to low_high_out with a cast
    for( int y = 0; y < _in_img.rows; y++ ){
        for( int x = 0; x < _in_img.cols; x++ ){
            for( int c = 0; c < ch_num; c++ ){

                if(_in_mask.empty()){
                    process_pixel = true;
                }else{
                    if (_in_mask.at<uchar>(y,x) != 0)
                        process_pixel = true;
                    else
                        process_pixel = false;
                }

                if (process_pixel) {
                    if (ch_num == 3){
                        _stretched_img.at<Vec3b>(y,x)[c] =
                                saturate_cast<uchar>( (_low_high_out.y - _low_high_out.x)*( _in_img.at<Vec3b>(y,x)[c] - _low_high_in.x )/(_low_high_in.y - _low_high_in.x) + _low_high_out.x );
                    }else{
                        _stretched_img.at<uchar>(y,x) =
                                saturate_cast<uchar>( (_low_high_out.y - _low_high_out.x)*( _in_img.at<uchar>(y,x) - _low_high_in.x )/(_low_high_in.y - _low_high_in.x) + _low_high_out.x );
                    }
                }else{
                    if (ch_num == 3){
                        _stretched_img.at<Vec3b>(y,x)[c] = _in_img.at<Vec3b>(y,x)[c];
                    }else{
                        _stretched_img.at<uchar>(y,x) = _in_img.at<uchar>(y,x);
                    }
                }
            }
        }
    }

}



void IMAGEPROCESSINGSHARED_EXPORT histogramQuantileStretch(Mat &_in_img, Mat &_in_mask, double _saturation_ratio, Mat &_stretched_img)
{

    int ch_num = _in_img.channels();
    bool process_pixel=false;

    vector<int> ch1_lim,ch2_lim,ch3_lim;
    vector<int> ch1_values,ch2_values,ch3_values;


    Mat *image_to_be_processed;
    Mat reduced_img;
    double alpha = sqrt( 1e5/((double)_in_img.cols*_in_img.rows) );

    if (alpha < 1.0){
        resize(_in_img,reduced_img,Size(), alpha, alpha);
        image_to_be_processed = &reduced_img;
    }else{
        image_to_be_processed = &_in_img;
    }


    // Stretches the image from low_high_in to low_high_out with a cast
    for( int y = 0; y < image_to_be_processed->rows; y++ ){
        for( int x = 0; x < image_to_be_processed->cols; x++ ){
            for( int c = 0; c < ch_num; c++ ){

                if(_in_mask.empty()){
                    process_pixel = true;
                }else{
                    if (_in_mask.at<uchar>(y,x) != 0)
                        process_pixel = true;
                    else
                        process_pixel = false;
                }

                if (process_pixel) {
                    switch (c) {
                    case 0:
                        ch1_values.push_back(image_to_be_processed->at<Vec3b>(y,x)[c]);
                        break;
                    case 1:
                        ch2_values.push_back(image_to_be_processed->at<Vec3b>(y,x)[c]);
                        break;
                    case 2:
                        ch3_values.push_back(image_to_be_processed->at<Vec3b>(y,x)[c]);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }

    // COnstruct required quantiles vector
    vector<double> quantiles;
    quantiles.push_back(_saturation_ratio);
    quantiles.push_back(1-_saturation_ratio);

    // Get channel saturation limits
    ch1_lim = integerQuantiles(ch1_values, quantiles);
    ch2_lim = integerQuantiles(ch2_values, quantiles);
    ch3_lim = integerQuantiles(ch3_values, quantiles);

    // Split img channels
    vector<Mat> tempRGB(3);
    vector<Mat> tempRGBout(3);
    cv::split(_in_img, tempRGB);

    // Stretches all channels
    Point ch1_low_high_in(ch1_lim[0],ch1_lim[1]);
    Point ch2_low_high_in(ch2_lim[0],ch2_lim[1]);
    Point ch3_low_high_in(ch3_lim[0],ch3_lim[1]);
    Point low_high_out(0,255);

    histogramStretch(tempRGB[0], _in_mask, ch1_low_high_in, low_high_out, tempRGBout[0]);
    histogramStretch(tempRGB[1], _in_mask, ch2_low_high_in, low_high_out, tempRGBout[1]);
    histogramStretch(tempRGB[2], _in_mask, ch3_low_high_in, low_high_out, tempRGBout[2]);

    // Merge channels
    merge(tempRGBout,_stretched_img);

}
