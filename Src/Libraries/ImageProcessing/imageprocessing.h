#ifndef IMAGE_PROCESSING_IMAGEPROCESSING_H_
#define IMAGE_PROCESSING_IMAGEPROCESSING_H_

#include <opencv2/opencv.hpp>
#include <vector>

namespace image_processing {

///
/// \brief histogramStretch streches img histogram from _low_high_in to _low_high_out
/// \param _in_img input image
/// \param _low_high_in input range
/// \param _low_high_out output range
/// \param _stretched_img streched image
///
void histogramStretch(const cv::Mat &_in_img, const cv::Point &_low_high_in, const cv::Point &_low_high_out, cv::Mat &_stretched_img, const bool _gamma_undo = true);

///
/// \brief histogramQuantileStretch
/// \param _in_img
/// \param _in_mask
/// \param _saturation_percentage
/// \param _stretched_img
///
void histogramQuantileStretch(const cv::Mat &_in_img, const cv::Mat &_in_mask, const double _saturation_percentage, cv::Mat &_stretched_img, const bool _gamma_undo=true);

void stretchColorImg(const cv::Mat& _in_img, const std::vector<int>& _ch1_lim, const std::vector<int>& _ch2_lim, const std::vector<int>& _ch3_lim, cv::Mat& _stretched_img, const bool _gamma_undo = true);

void findImgColorQuantiles(const cv::Mat& _in_img, const cv::Mat& _in_mask, std::vector<double> &_quantiles, std::vector<int> &_ch1_lim, std::vector<int> &_ch2_lim, std::vector<int> &_ch3_lim);

void findImgQuantiles(const cv::Mat& _in_img, const cv::Mat& _in_mask, std::vector<double>& _quantiles, std::vector<int>& _ch_lim);

/// <summary>
/// Gamma transformation from linear image to rgb
/// </summary>
/// <param name="_lin">linear value</param>
/// <returns>rgb value</returns>
double lin2rgbf(const double _lin);

/// <summary>
/// Remove gamma transformation from rgb to linear
/// </summary>
/// <param name="_lin">linear value</param>
/// <returns>rgb value</returns>
double rgb2linf(const double _rgb);


double gamma_do(const double _input, const double _gamma_value=5.0);

double gamma_undo(const double _input, const double _gamma_value=5.0);

} // namespace image_processing

#endif // IMAGE_PROCESSING_IMAGEPROCESSING_H_
