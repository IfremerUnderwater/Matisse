#include "imageprocessing.h"
#include "stdvectoperations.h"
#include <math.h>

using namespace cv;
using namespace std;
using namespace basic_processing;

namespace image_processing {

void histogramStretch(Mat &_in_img, Point _low_high_in, Point _low_high_out,  Mat &_stretched_img, bool _gamma_undo)
{


    // Init
    _stretched_img = Mat::zeros( _in_img.size(), _in_img.type() );
    int ch_num = _in_img.channels();
    bool process_pixel=true;

    /// Stretches the image from low_high_in to low_high_out with a cast
    for( int y = 0; y < _in_img.rows; y++ ){
        for( int x = 0; x < _in_img.cols; x++ ){
            for( int c = 0; c < ch_num; c++ ){

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
    stretchColorImg(_in_img, ch1_lim, ch2_lim, ch3_lim, _stretched_img, _gamma_undo);

}

void stretchColorImg(cv::Mat& _in_img, std::vector<int>& _ch1_lim, std::vector<int>& _ch2_lim, std::vector<int>& _ch3_lim, cv::Mat& _stretched_img, bool _gamma_undo)
{
    // Split img channels
    vector<Mat> temp_rgb(3);
    vector<Mat> temp_rgb_out(3);
    cv::split(_in_img, temp_rgb);

    // Stretches all channels
    Point ch1_low_high_in(_ch1_lim[0], _ch1_lim[1]);
    Point ch2_low_high_in(_ch2_lim[0], _ch2_lim[1]);
    Point ch3_low_high_in(_ch3_lim[0], _ch3_lim[1]);
    Point low_high_out(0, 255);

    histogramStretch(temp_rgb[0], ch1_low_high_in, low_high_out, temp_rgb_out[0], _gamma_undo);
    histogramStretch(temp_rgb[1], ch2_low_high_in, low_high_out, temp_rgb_out[1], _gamma_undo);
    histogramStretch(temp_rgb[2], ch3_low_high_in, low_high_out, temp_rgb_out[2], _gamma_undo);

    // Merge channels
    merge(temp_rgb_out, _stretched_img);
}

void findImgColorQuantiles(const cv::Mat& _in_img, const cv::Mat& _in_mask, vector<double>& _quantiles, vector<int>& _ch1_lim, vector<int>& _ch2_lim, vector<int>& _ch3_lim)
{
    int ch_num = _in_img.channels();
    if (ch_num < 3)
        return findImgQuantiles(_in_img, _in_mask, _quantiles, _ch1_lim);

    // No need to compute sqrt as we are only interested in the alpha coef
    // being less than 1.0 or not
    // double alpha = sqrt(1e6 / ((double)_in_img.cols * _in_img.rows));
    const double alpha = 1e6 / ((double)_in_img.cols * _in_img.rows);

    cv::Mat reduced_img;

    if (alpha < 1.0) {
        cv::resize(_in_img, reduced_img, cv::Size(), alpha, alpha);
    }
    else {
        reduced_img = _in_img;
    }

    std::vector<int> ch1_values, ch2_values, ch3_values;
    ch1_values.reserve(reduced_img.rows * reduced_img.cols);
    ch2_values.reserve(reduced_img.rows * reduced_img.cols);
    ch3_values.reserve(reduced_img.rows * reduced_img.cols);
    
    // Stretches the image from low_high_in to low_high_out with a cast
    // If a mask is to be used, do not consider masked pixels
    if (_in_mask.empty())
    {
        for (int r = 0; r < reduced_img.rows; r++)
        {
            const Vec3b* img_row = reduced_img.ptr<Vec3b>(r);
            for(int c = 0; c < reduced_img.cols; c++)
            {
                ch1_values.push_back(img_row[c][0]);
                ch2_values.push_back(img_row[c][1]);
                ch3_values.push_back(img_row[c][2]);
            }
        } 
    }
    else 
    {
        for (int r = 0; r < reduced_img.rows; r++)
        {
            const Vec3b* img_row = reduced_img.ptr<Vec3b>(r);
            const uchar* mask_row = _in_mask.ptr<uchar>(r);

            for(int c = 0; c < reduced_img.cols; c++)
            {
                if (mask_row[c] != 0)
                {
                    ch1_values.push_back(img_row[c][0]);
                    ch2_values.push_back(img_row[c][1]);
                    ch3_values.push_back(img_row[c][2]);
                }
            }
        }
    }

    // Get channel saturation limits
    _ch1_lim = integerQuantiles(ch1_values, _quantiles);
    _ch2_lim = integerQuantiles(ch2_values, _quantiles);
    _ch3_lim = integerQuantiles(ch3_values, _quantiles);
}

void findImgQuantiles(const cv::Mat& _in_img, const cv::Mat& _in_mask, std::vector<double>& _quantiles, std::vector<int>& _ch_lim)
{
    // No need to compute sqrt as we are only interested in the alpha coef
    // being less than 1.0 or not
    // double alpha = sqrt(1e6 / ((double)_in_img.cols * _in_img.rows));
    const double alpha = 1e6 / ((double)_in_img.cols * _in_img.rows);

    cv::Mat reduced_img;

    if (alpha < 1.0) {
        cv::resize(_in_img, reduced_img, cv::Size(), alpha, alpha);
    }
    else {
        reduced_img = _in_img;
    }

    std::vector<int> ch_values;
    ch_values.reserve(reduced_img.rows * reduced_img.cols);
    
    // Stretches the image from low_high_in to low_high_out with a cast
    // If a mask is to be used, do not consider masked pixels
    if (_in_mask.empty())
    {
        for (int r = 0; r < reduced_img.rows; r++)
        {
            const uchar* img_row = reduced_img.ptr<uchar>(r);
            for(int c = 0; c < reduced_img.cols; c++)
                ch_values.push_back(img_row[c]);
        } 
    }
    else 
    {
        for (int r = 0; r < reduced_img.rows; r++)
        {
            const uchar* img_row = reduced_img.ptr<uchar>(r);
            const uchar* mask_row = _in_mask.ptr<uchar>(r);

            for(int c = 0; c < reduced_img.cols; c++)
            {
                if (mask_row[c] != 0)
                    ch_values.push_back(img_row[c]);
            }
        }
    }

    // Get channel saturation limits
    _ch_lim = integerQuantiles(ch_values, _quantiles);
}

double lin2rgbf(double _rgb)
{
    // 1.0/2.19921875 = 0.45470692
    return pow(_rgb, 0.45470692);
}

double rgb2linf(double _lin)
{
    // gamma = 2.19921875
    return pow(_lin,2.19921875);
}


double gamma_do(double _input, double _gamma_value)
{
    return pow(_input, _gamma_value);
}

double gamma_undo(double _input, double _gamma_value)
{
    return pow(_input, 1.0/_gamma_value);
}

} // namespace image_processing
