#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include <opencv2/opencv.hpp>
#include <vector>

///
/// \brief histogramStretch streches img histogram from _low_high_in to _low_high_out
/// \param _in_img input image
/// \param _in_mask image mask (process only non zero values)
/// \param _low_high_in input range
/// \param _low_high_out output range
/// \param _stretched_img streched image
///
void histogramStretch(cv::Mat &_in_img, cv::Mat &_in_mask, cv::Point _low_high_in, cv::Point _low_high_out,  cv::Mat &_stretched_img, bool _gamma_undo = true);

///
/// \brief histogramQuantileStretch
/// \param _in_img
/// \param _in_mask
/// \param _saturation_percentage
/// \param _stretched_img
///
void histogramQuantileStretch(cv::Mat &_in_img, cv::Mat &_in_mask, double _saturation_percentage,  cv::Mat &_stretched_img, bool _gamma_undo=true);

void stretchColorImg(cv::Mat& _in_img, cv::Mat& _in_mask, std::vector<int>& _ch1_lim, std::vector<int>& _ch2_lim, std::vector<int>& _ch3_lim, cv::Mat& _stretched_img, bool _gamma_undo = true);
void findImgColorQuantiles(cv::Mat& _in_img, cv::Mat& _in_mask, std::vector<double> &_quantiles, std::vector<int> &_ch1_lim, std::vector<int> &_ch2_lim, std::vector<int> &_ch3_lim);

void findImgQuantiles(cv::Mat& _in_img, cv::Mat& _in_mask, std::vector<double>& _quantiles, std::vector<int>& _ch_lim);

/// <summary>
/// Gamma transformation from linear image to rgb
/// </summary>
/// <param name="_lin">linear value</param>
/// <returns>rgb value</returns>
double lin2rgbf(double _lin);

/// <summary>
/// Remove gamma transformation from rgb to linear
/// </summary>
/// <param name="_lin">linear value</param>
/// <returns>rgb value</returns>
double rgb2linf(double _rgb);


double gamma_do(double _input, double _gamma_value=5.0);

double gamma_undo(double _input, double _gamma_value=5.0);

#endif // IMAGEPROCESSING_H
