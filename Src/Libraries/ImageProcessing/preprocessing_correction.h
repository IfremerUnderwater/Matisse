#ifndef PREPROCESSING_CORRECTION_H
#define PREPROCESSING_CORRECTION_H

#include <opencv2/core/mat.hpp>
#include <deque>
#include <vector>
#include <string>

#pragma once
class PreprocessingCorrection
{
public:

	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="_ws">ws is the sliding window size (number of image used for the computation of the model)</param>
	PreprocessingCorrection(int _ws=5);

	/// <summary>
	/// Correct images for illumination given a list of files on the disk
	/// </summary>
	/// <param name="_input_img_files">files path list</param>
	/// <param name="_output_path">output folder path</param>
	/// <returns>true on success</returns>
	bool correctImageList(std::vector<std::string> _input_img_files, std::string _output_path);

	/// <summary>
	/// Compute median image in the sliding window
	/// </summary>
	/// <returns>true on success</returns>
	bool computeTemporalMedian();

	/// <summary>
	/// Compute pattern model and compensate illumination
	/// </summary>
	/// <param name="_input_image">Input image</param>
	/// <param name="_output_image">Compensated output image</param>
	/// <param name="_temporal_median_image">Temporal median image</param>
	/// <returns>true on success</returns>
	bool compensateIllumination(cv::Mat& _input_image, cv::Mat& _output_image, Mat& _temporal_median_image);



private:
	int m_ws;
	double m_scaling_factor;

	cv::Mat m_blue_median_img;
	cv::Mat m_green_median_img;
	cv::Mat m_red_median_img;

	std::deque<cv::Mat> m_blue_stack_images;
	std::deque<cv::Mat> m_green_stack_images;
	std::deque<cv::Mat> m_red_stack_images;

};

#endif // PREPROCESSING_CORRECTION_H
