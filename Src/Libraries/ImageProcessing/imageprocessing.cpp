#include "imageprocessing.h"
#include "stdvectoperations.h"
#include <math.h>

#include <omp.h>

using namespace cv;
using namespace std;
using namespace basic_processing;

namespace image_processing {

void histogramStretch(const cv::Mat &_in_img, const cv::Point &_low_high_in, const cv::Point &_low_high_out,  cv::Mat &_stretched_img, const bool _gamma_undo)
{
    // Init
    _stretched_img = Mat::zeros( _in_img.size(), _in_img.type() );
    
    if (abs(_low_high_in.y - _low_high_in.x) < 1)
    {
        _stretched_img = _in_img.clone();
        return;
    }

    // This should not happen as it would mean that we use the same stretching coefficients
    // for every color channels.
    if (_in_img.channels() == 3)
    {
        std::vector<cv::Mat> vtmp_bgr_img, vtmp_bgr_img_out;
        cv::split(_in_img, vtmp_bgr_img);
        vtmp_bgr_img_out.resize(3);
        for (int i = 0; i < 3; ++i)
        {
            histogramStretch(vtmp_bgr_img[i], _low_high_in, _low_high_out, vtmp_bgr_img_out[i], _gamma_undo);
        }
        cv::merge(vtmp_bgr_img_out, _stretched_img);
        return;
    }


    // Pre-compute low/high limits between 0.0 and 1.0
    cv::Point2d low_high_in, low_high_out;

    const double inv_max_val = 1. / 255.0;
    
    low_high_in.x = static_cast<double>(_low_high_in.x) * inv_max_val;
    low_high_in.y = static_cast<double>(_low_high_in.y) * inv_max_val;
    low_high_out.x = static_cast<double>(_low_high_out.x) * inv_max_val;
    low_high_out.y = static_cast<double>(_low_high_out.y) * inv_max_val;

    const double low_high_coef = (low_high_out.y - low_high_out.x) / (low_high_in.y - low_high_in.x);


    // undo gamma if needed
    if (_gamma_undo)
    {
        low_high_in.x = rgb2linf(low_high_in.x);
        low_high_in.y = rgb2linf(low_high_in.y);
        low_high_out.x = rgb2linf(low_high_out.x);
        low_high_out.y = rgb2linf(low_high_out.y);
    }

    // Set-up LUT to store the mapping to apply for each intensity value
    cv::Mat look_up_table = cv::Mat(1,256,CV_8U);
    uchar* lut_val = look_up_table.data;

    // Compute the new intensity value to apply for each initial intensity
    #pragma omp parallel for // Max: Should be of use when illumination compensation is required but no noticeable effect on my machine (doesn't hurt neither so I leave it here)
    for (int i = 0 ; i < 256 ; i++)
    {
        double current_intensity = static_cast<double>(i) * inv_max_val;

        if (_gamma_undo)
        {
            current_intensity = rgb2linf(current_intensity);
        }

        const double out_intensity = low_high_coef * (current_intensity - low_high_in.x) + low_high_out.x;
        
        if (_gamma_undo)
            lut_val[i] = cv::saturate_cast<uchar>(255.0 * lin2rgbf(out_intensity));
        else
            lut_val[i] = cv::saturate_cast<uchar>(255.0 * out_intensity);
    }

    // Apply the intensity mapping
    cv::LUT(_in_img, look_up_table, _stretched_img);
}



void histogramQuantileStretch(const cv::Mat &_in_img, const cv::Mat &_in_mask, const double _saturation_ratio, cv::Mat &_stretched_img, const bool _gamma_undo)
{

    // Construct required quantiles vector
    std::vector<double> quantiles;
    quantiles.push_back(_saturation_ratio);
    quantiles.push_back(1-_saturation_ratio);

    // Get channel saturation limits
    std::vector<int> ch1_lim, ch2_lim, ch3_lim;
    findImgColorQuantiles(_in_img, _in_mask, quantiles, ch1_lim, ch2_lim, ch3_lim);

    // Strech img according to saturation limit
    stretchColorImg(_in_img, ch1_lim, ch2_lim, ch3_lim, _stretched_img, _gamma_undo);
}

void stretchColorImg(const cv::Mat& _in_img, const std::vector<int>& _ch1_lim, const std::vector<int>& _ch2_lim, const std::vector<int>& _ch3_lim, cv::Mat& _stretched_img, const bool _gamma_undo)
{
    // Split img channels
    std::vector<cv::Mat> temp_rgb(3);
    std::vector<cv::Mat> temp_rgb_out(3);
    cv::split(_in_img, temp_rgb);

    // Stretches all channels
    const cv::Point ch1_low_high_in(_ch1_lim[0], _ch1_lim[1]);
    const cv::Point ch2_low_high_in(_ch2_lim[0], _ch2_lim[1]);
    const cv::Point ch3_low_high_in(_ch3_lim[0], _ch3_lim[1]);
    const cv::Point low_high_out(0, 255);

    histogramStretch(temp_rgb[0], ch1_low_high_in, low_high_out, temp_rgb_out[0], _gamma_undo);
    histogramStretch(temp_rgb[1], ch2_low_high_in, low_high_out, temp_rgb_out[1], _gamma_undo);
    histogramStretch(temp_rgb[2], ch3_low_high_in, low_high_out, temp_rgb_out[2], _gamma_undo);

    // Merge channels
    cv::merge(temp_rgb_out, _stretched_img);
}

void findImgColorQuantiles(const cv::Mat& _in_img, const cv::Mat& _in_mask, vector<double>& _quantiles, vector<int>& _ch1_lim, vector<int>& _ch2_lim, vector<int>& _ch3_lim)
{
    int ch_num = _in_img.channels();
    if (ch_num < 3)
        return findImgQuantiles(_in_img, _in_mask, _quantiles, _ch1_lim);

    const double alpha = sqrt(1e6 / ((double)_in_img.cols * _in_img.rows));

    cv::Mat reduced_img, reduced_mask;

    if (alpha < 1.0) {
        cv::resize(_in_img, reduced_img, cv::Size(), alpha, alpha);

        if (!_in_mask.empty()) {
            // Nearest neighb interpolation used to keep 0 or 255 values only
            cv::resize(_in_mask, reduced_mask, cv::Size(), alpha, alpha, cv::INTER_NEAREST);
        }
    }
    else {
        reduced_img = _in_img;
        reduced_mask = _in_mask;
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
            const uchar* mask_row = reduced_mask.ptr<uchar>(r);

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
    const double alpha = sqrt(1e6 / ((double)_in_img.cols * _in_img.rows));

    cv::Mat reduced_img, reduced_mask;

    if (alpha < 1.0) {
        cv::resize(_in_img, reduced_img, cv::Size(), alpha, alpha);

        if (!_in_mask.empty()) {
            // Nearest neighb interpolation used to keep 0 or 255 values only
            cv::resize(_in_mask, reduced_mask, cv::Size(), alpha, alpha, cv::INTER_NEAREST);
        }
    }
    else {
        reduced_img = _in_img;
        reduced_mask = _in_mask;
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
            const uchar* mask_row = reduced_mask.ptr<uchar>(r);

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

double lin2rgbf(const double _rgb)
{
    // 1.0/2.19921875 = 0.45470692
    return pow(_rgb, 0.45470692);
}

double rgb2linf(const double _lin)
{
    // gamma = 2.19921875
    return pow(_lin,2.19921875);
}


double gamma_do(const double _input, const double _gamma_value)
{
    return pow(_input, _gamma_value);
}

double gamma_undo(const double _input, const double _gamma_value)
{
    return pow(_input, 1.0/_gamma_value);
}

} // namespace image_processing
