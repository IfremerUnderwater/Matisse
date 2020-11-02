#include "imageprocessing.h"
#include "stdvectoperations.h"
#include <math.h>

using namespace cv;

void histogramStretch(Mat &_in_img, Mat &_in_mask, Point _low_high_in, Point _low_high_out,  Mat &_stretched_img, bool _gamma_undo)
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

                if (process_pixel & (abs(_low_high_in.y - _low_high_in.x) >=1) ) {

                    // we compute all in double then saturate cast
                    double current_intensity, out_intensity;
                    Point2d low_high_in, low_high_out;

                    if (ch_num == 3)
                        current_intensity = static_cast<double>( _in_img.at<Vec3b>(y, x)[c] ) / 255.0;
                    else
                        current_intensity = static_cast<double>( _in_img.at<uchar>(y, x) ) / 255.0;

                    low_high_in.x = static_cast<double>(_low_high_in.x) / 255.0;
                    low_high_in.y = static_cast<double>(_low_high_in.y) / 255.0;
                    low_high_out.x = static_cast<double>(_low_high_out.x) / 255.0;
                    low_high_out.y = static_cast<double>(_low_high_out.y) / 255.0;

                    // undo gamma if needed
                    if (_gamma_undo)
                    {
                        low_high_in.x = rgb2linf(low_high_in.x);
                        low_high_in.y = rgb2linf(low_high_in.y);
                        low_high_out.x = rgb2linf(low_high_out.x);
                        low_high_out.y = rgb2linf(low_high_out.y);
                        current_intensity = rgb2linf(current_intensity);
                    }

                    out_intensity = (low_high_out.y - low_high_out.x) * (current_intensity - low_high_in.x) / (low_high_in.y - low_high_in.x) + low_high_out.x;

                    if (ch_num == 3){
                        if (_gamma_undo)
                            _stretched_img.at<Vec3b>(y, x)[c] = saturate_cast<uchar>(255.0 * lin2rgbf(out_intensity));
                        else
                            _stretched_img.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( 255.0*out_intensity );
                    }else{
                        if (_gamma_undo)
                            _stretched_img.at<uchar>(y, x) = saturate_cast<uchar>(255.0 * lin2rgbf(out_intensity));
                        else
                            _stretched_img.at<uchar>(y, x) = saturate_cast<uchar>(255.0 * out_intensity);
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



void histogramQuantileStretch(Mat &_in_img, Mat &_in_mask, double _saturation_ratio, Mat &_stretched_img, bool _gamma_undo)
{

    // Construct required quantiles vector
    vector<double> quantiles;
    quantiles.push_back(_saturation_ratio);
    quantiles.push_back(1-_saturation_ratio);

    // Get channel saturation limits
    vector<int> ch1_lim, ch2_lim, ch3_lim;
    findImgColorQuantiles(_in_img, _in_mask, quantiles, ch1_lim, ch2_lim, ch3_lim);

    // Strech img according to saturation limit
    stretchColorImg(_in_img, _in_mask, ch1_lim, ch2_lim, ch3_lim, _stretched_img, _gamma_undo);

}

void stretchColorImg(cv::Mat& _in_img, cv::Mat& _in_mask, std::vector<int>& _ch1_lim, std::vector<int>& _ch2_lim, std::vector<int>& _ch3_lim, cv::Mat& _stretched_img, bool _gamma_undo)
{
    // Split img channels
    vector<Mat> tempRGB(3);
    vector<Mat> tempRGBout(3);
    cv::split(_in_img, tempRGB);

    // Stretches all channels
    Point ch1_low_high_in(_ch1_lim[0], _ch1_lim[1]);
    Point ch2_low_high_in(_ch2_lim[0], _ch2_lim[1]);
    Point ch3_low_high_in(_ch3_lim[0], _ch3_lim[1]);
    Point low_high_out(0, 255);

    histogramStretch(tempRGB[0], _in_mask, ch1_low_high_in, low_high_out, tempRGBout[0], _gamma_undo);
    histogramStretch(tempRGB[1], _in_mask, ch2_low_high_in, low_high_out, tempRGBout[1], _gamma_undo);
    histogramStretch(tempRGB[2], _in_mask, ch3_low_high_in, low_high_out, tempRGBout[2], _gamma_undo);

    // Merge channels
    merge(tempRGBout, _stretched_img);
}

void findImgColorQuantiles(cv::Mat& _in_img, cv::Mat& _in_mask, vector<double>& _quantiles, vector<int>& _ch1_lim, vector<int>& _ch2_lim, vector<int>& _ch3_lim)
{
    int ch_num = _in_img.channels();
    bool process_pixel = false;

    vector<int> ch1_values, ch2_values, ch3_values;

    Mat* image_to_be_processed;
    Mat reduced_img;
    double alpha = sqrt(1e6 / ((double)_in_img.cols * _in_img.rows));

    if (alpha < 1.0) {
        resize(_in_img, reduced_img, Size(), alpha, alpha);
        image_to_be_processed = &reduced_img;
    }
    else {
        image_to_be_processed = &_in_img;
    }


    // Stretches the image from low_high_in to low_high_out with a cast
    for (int y = 0; y < image_to_be_processed->rows; y++) {
        for (int x = 0; x < image_to_be_processed->cols; x++) {
            for (int c = 0; c < ch_num; c++) {

                if (_in_mask.empty()) {
                    process_pixel = true;
                }
                else {
                    if (_in_mask.at<uchar>(y, x) != 0)
                        process_pixel = true;
                    else
                        process_pixel = false;
                }

                if (process_pixel) {
                    switch (c) {
                    case 0:
                        ch1_values.push_back(image_to_be_processed->at<Vec3b>(y, x)[c]);
                        break;
                    case 1:
                        ch2_values.push_back(image_to_be_processed->at<Vec3b>(y, x)[c]);
                        break;
                    case 2:
                        ch3_values.push_back(image_to_be_processed->at<Vec3b>(y, x)[c]);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }

    // Get channel saturation limits
    _ch1_lim = integerQuantiles(ch1_values, _quantiles);
    _ch2_lim = integerQuantiles(ch2_values, _quantiles);
    _ch3_lim = integerQuantiles(ch3_values, _quantiles);
}

double lin2rgbf(double _lin)
{
    // gamma = 2.19921875
    return pow(_lin,2.19921875);
}

double rgb2linf(double _rgb)
{
    // 1.0/2.19921875 = 0.45470692
    return pow(_rgb, 0.45470692);
}
