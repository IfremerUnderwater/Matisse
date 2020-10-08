#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include <opencv2/opencv.hpp>

///
/// \brief histogramStretch streches img histogram from _low_high_in to _low_high_out
/// \param _in_img input image
/// \param _in_mask image mask (process only non zero values)
/// \param _low_high_in input range
/// \param _low_high_out output range
/// \param _stretched_img streched image
///
void histogramStretch(cv::Mat &_in_img, cv::Mat &_in_mask, cv::Point _low_high_in, cv::Point _low_high_out,  cv::Mat &_stretched_img);

///
/// \brief histogramQuantileStretch
/// \param _in_img
/// \param _in_mask
/// \param _saturation_percentage
/// \param _stretched_img
///
void histogramQuantileStretch(cv::Mat &_in_img, cv::Mat &_in_mask, double _saturation_percentage,  cv::Mat &_stretched_img);

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

#endif // IMAGEPROCESSING_H
