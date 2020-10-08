#include "preprocessing_correction.h"
#include "imageprocessing.h"
#include "stdvectoperations.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

PreprocessingCorrection::PreprocessingCorrection(int _ws) :m_ws(_ws), m_scaling_factor(1.0)
{
}

bool PreprocessingCorrection::correctImageList(std::vector<std::string> _input_img_files, std::string _output_path)
{
	// check if we need to reduce image resolution
	cv::Mat first_img = cv::imread(_input_img_files[0], cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
	double m_scaling_factor = sqrt(1e5 / ((double)first_img.cols * first_img.rows)); // process image at maximum 1Mpx
	if (m_scaling_factor > 1.0)
		m_scaling_factor = 1.0;
	//resize(_in_img, reduced_img, Size(), alpha, alpha);

	if (_input_img_files.size() < m_ws)
		return false;

	int im_nb = _input_img_files.size();
	int half_ws = (m_ws-1)/2;

	for (int i = 0; i < im_nb; i++)
	{
		Mat current_img = imread(_input_img_files[i], cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);

		if (i-half_ws <= 0)
		{

			if (i == 0)
			{

				for (int j = 0; j < m_ws; j++)
				{
					Mat temp_img;
					resize(imread(_input_img_files[j], cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION), temp_img, Size(), m_scaling_factor, m_scaling_factor);

					vector<Mat> temp_BRG(3);
					split(temp_img, temp_BRG);

					// stacking images for median
					m_blue_stack_images.push_back(temp_BRG[0]);
					m_green_stack_images.push_back(temp_BRG[1]);
					m_red_stack_images.push_back(temp_BRG[2]);

				}
			}

		}
		else if ( i+half_ws < im_nb-1)
		{
			Mat temp_img;
			resize(imread(_input_img_files[i+half_ws], cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION), temp_img, Size(), m_scaling_factor, m_scaling_factor);


			vector<Mat> temp_BRG(3);
			split(temp_img, temp_BRG);

			// stacking images for median
			m_blue_stack_images.pop_front();
			m_blue_stack_images.push_back(temp_BRG[0]);
			m_green_stack_images.pop_front();
			m_green_stack_images.push_back(temp_BRG[1]);
			m_red_stack_images.pop_front();
			m_red_stack_images.push_back(temp_BRG[2]);

		}

	}

	if (!computeMedian())
		return false;

	return true;
}

bool PreprocessingCorrection::computeMedian()
{
	// check we have at least one image for correction
	if (m_blue_stack_images.size() < 1)
		return false;
	if (m_green_stack_images.size() < 1)
		return false;
	if (m_red_stack_images.size() < 1)
		return false;

	// TODO check type, we only support color 8bits images for the moment

	// compute median
	Size images_size = m_blue_stack_images[0].size();
	int stack_size = m_blue_stack_images.size();
	m_blue_median_img = Mat::zeros(images_size, CV_32FC1);
	m_green_median_img = Mat::zeros(images_size, CV_32FC1);
	m_red_median_img = Mat::zeros(images_size, CV_32FC1);

	vector<double> blue_pixel_stack; blue_pixel_stack.reserve(stack_size);
	vector<double> green_pixel_stack; green_pixel_stack.reserve(stack_size);
	vector<double> red_pixel_stack; red_pixel_stack.reserve(stack_size);

	for (int w = 0; w < images_size.width; w++)
	{
		for (int h = 0; w < images_size.height; w++)
		{
			for (int k = 0; k < m_blue_stack_images.size(); k++)
			{
				blue_pixel_stack[k] = rgb2linf(static_cast<double>(m_blue_stack_images[k].at<uchar>(w, h)) / 255.0);
				green_pixel_stack[k] = rgb2linf(static_cast<double>(m_green_stack_images[k].at<uchar>(w, h)) / 255.0);
				red_pixel_stack[k] = rgb2linf(static_cast<double>(m_red_stack_images[k].at<uchar>(w, h)) / 255.0);
			}
			double tempMedian;

			tempMedian = doubleVectorMedian(blue_pixel_stack);
			m_blue_median_img.at<float>(w, h) = static_cast<float>(tempMedian);

			tempMedian = doubleVectorMedian(green_pixel_stack);
			m_green_median_img.at<float>(w, h) = static_cast<float>(tempMedian);

			tempMedian = doubleVectorMedian(red_pixel_stack);
			m_red_median_img.at<float>(w, h) = static_cast<float>(tempMedian);

		}
	}

	return true;
}

bool PreprocessingCorrection::compensateIllumination(Mat& _input_image, Mat& _output_image)
{

}
