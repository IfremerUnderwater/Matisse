#include "preprocessing_correction.h"
#include "imageprocessing.h"
#include "stdvectoperations.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <Eigen/dense>
#include <QFileInfo>
#include <QDir>

using namespace std;
using namespace cv;
using namespace Eigen;

PreprocessingCorrection::PreprocessingCorrection(int _ws) :m_ws(_ws), 
m_median_comp_scaling(1.0),
m_prepro_img_scaling(1.0),
m_correct_colors(false),
m_compensate_illumination(false)
{
}

bool PreprocessingCorrection::preprocessImageList(QStringList _input_img_files, QString _output_path)
{
	// check if we need to reduce image resolution
	cv::Mat first_img = cv::imread(_input_img_files[0].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
	double m_median_comp_scaling = sqrt(1e5 / ((double)first_img.cols * first_img.rows)); // process image at maximum 1Mpx
	if (m_median_comp_scaling > 1.0)
		m_median_comp_scaling = 1.0;
	//resize(_in_img, reduced_img, Size(), alpha, alpha);

	if (_input_img_files.size() < m_ws)
		return false;

	int im_nb = _input_img_files.size();
	int half_ws = (m_ws-1)/2;

	// preprocess
	for (int i = 0; i < im_nb; i++)
	{
		Mat current_img = imread(_input_img_files[i].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);

		// check if we need to resize image
		if (m_prepro_img_scaling < 1.0)
			resize(current_img, current_img, Size(), m_prepro_img_scaling, m_prepro_img_scaling);

		// need illumination compensation
		if (m_compensate_illumination)
		{
			if (i - half_ws <= 0)
			{

				if (i == 0)
				{

					for (int j = 0; j < m_ws; j++)
					{
						Mat temp_img;
						resize(imread(_input_img_files[j].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION), temp_img, Size(), m_median_comp_scaling, m_median_comp_scaling);

						vector<Mat> temp_BRG(3);
						split(temp_img, temp_BRG);

						// stacking images for median
						m_blue_stack_images.push_back(temp_BRG[0]);
						m_green_stack_images.push_back(temp_BRG[1]);
						m_red_stack_images.push_back(temp_BRG[2]);

					}
				}

			}
			else if (i + half_ws < im_nb - 1)
			{
				Mat temp_img;
				resize(imread(_input_img_files[i + half_ws].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION), temp_img, Size(), m_median_comp_scaling, m_median_comp_scaling);


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

			// Compute temporal median
			if (!computeTemporalMedian())
				return false;

			vector<Mat> current_BRG(3);
			vector<Mat> current_BRG_corr(3);
			split(current_img, current_BRG);

			// compute illum correction
			compensateIllumination(current_BRG[0], current_BRG_corr[0], m_blue_median_img);
			compensateIllumination(current_BRG[1], current_BRG_corr[1], m_green_median_img);
			compensateIllumination(current_BRG[2], current_BRG_corr[2], m_red_median_img);

			merge(current_BRG_corr, current_img);

		} // end need illumination compensation

		// correct colors
		if (m_correct_colors)
		{
			cv::Mat empty_mask;
			histogramQuantileStretch(current_img, empty_mask, 0.0005, current_img);
		}

		// write image
		QFileInfo current_file_info(_input_img_files[i]);
		QString outfile = _output_path + QDir::separator() + current_file_info.fileName();
		cv::imwrite(outfile.toStdString(), current_img);

	}

	return true;
}

bool PreprocessingCorrection::computeTemporalMedian()
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
		for (int h = 0; h < images_size.height; h++)
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

bool PreprocessingCorrection::compensateIllumination(Mat& _input_image, Mat& _output_image, Mat& _temporal_median_image)
{
	// spatially filter temporal median image
	Mat _temporal_spatial_median_image;
	std::vector<double> _temporal_spatial_median_vect;
	medianBlur(_temporal_median_image, _temporal_spatial_median_image, 5);

	double out_perc = 0.01; // outlier percentage
	double maximum_corr_factor = 10.0; // if correction is greater we truncate
	vector<double> channel_limits;
	vector<double> quantiles;
	quantiles.push_back(out_perc);
	quantiles.push_back(1 - out_perc);

	for (int w = 0; w < _temporal_spatial_median_image.rows; w++)
	{
		for (int h = 0; h < _temporal_spatial_median_image.cols; h++)
		{
			_temporal_spatial_median_vect.push_back(_temporal_spatial_median_image.at<float>(w, h));
		}
	}
	channel_limits = doubleQuantiles(_temporal_spatial_median_vect, quantiles);

	// Construct x,y,z fitting dataset (x = width, y = height, z=intensity) for paraboloid model
	vector<double> x, y, z;
	double temp_z;
	for (int w = 0; w < _temporal_spatial_median_image.rows; w++)
	{
		for (int h = 0; h < _temporal_spatial_median_image.cols; h++)
		{
			temp_z = _temporal_spatial_median_image.at<float>(w, h);
			if (temp_z > channel_limits[0] && temp_z < channel_limits[1])
			{
				x.push_back(w);
				y.push_back(h);
				z.push_back(temp_z);
			}
		}
	}

	// Solve paraboloid model
	MatrixXf A = MatrixXf::Zero(x.size(),10);
	VectorXf b = VectorXf::Zero(z.size());
	
	for (int i = 0; i < z.size(); i++)
	{
		// A = [x. ^ 3, y. ^ 3, x.*y. ^ 2, x. ^ 2. * y, x. ^ 2, y. ^ 2, x.*y, x, y, ones(size(x))];
		// idx  0       1       2          3            4       5       6     7  8  9
		A(i, 0) = pow(x[i], 3);
		A(i, 1) = pow(y[i], 3);
		A(i, 2) = x[i]*pow(y[i], 2);
		A(i, 3) = pow(x[i],2)*y[i];
		A(i, 4) = pow(x[i], 2);
		A(i, 5) = pow(y[i], 2);
		A(i, 6) = x[i]*y[i];
		A(i, 7) = x[i];
		A(i, 8) = y[i];

		b(i) = z[i];

	}

	VectorXf alpha;
	alpha = A.bdcSvd(ComputeThinU | ComputeThinV).solve(b); // solve A*alpha = b

	// Correct image
	_output_image = _input_image;
	double temp_x, temp_y, corr_factor;
	for (int w = 0; w < _input_image.rows; w++)
	{
		for (int h = 0; h < _input_image.cols; h++)
		{
			temp_x = static_cast<double>(w)*m_median_comp_scaling/m_prepro_img_scaling;
			temp_y = static_cast<double>(h)*m_median_comp_scaling/m_prepro_img_scaling;
			temp_z = alpha(0)*pow(temp_x,3) + alpha(1)*pow(temp_y, 3) + alpha(2)*temp_x*pow(temp_y, 2)
				+ alpha(3)*pow(temp_x, 2)*temp_y + alpha(4)*pow(temp_x, 2) + alpha(5)*pow(temp_y, 2)
				+ alpha(6)*temp_x*temp_y + alpha(7)*temp_x + alpha(8)*temp_y + alpha(9);

			if (temp_z > 1.0 / maximum_corr_factor || temp_z < 0)
			{
				corr_factor = maximum_corr_factor;
			}
			else
			{
				corr_factor = 1.0 / temp_z;
			}

			// correct accounting for gamma factor
			_output_image.at<uchar>(w,h) = static_cast<uchar>( 255*lin2rgbf( corr_factor*rgb2linf(static_cast<double>(_input_image.at<uchar>(w, h)) / 255.0) ) );
		}
	}

}

void PreprocessingCorrection::configureProcessing(bool _correct_colors, bool _compensate_illumination, double _prepro_img_scaling)
{
	m_correct_colors = _correct_colors;
	m_compensate_illumination = _compensate_illumination;
	if (_prepro_img_scaling > 1.0)
		m_prepro_img_scaling = 1.0;
	else
		m_prepro_img_scaling = _prepro_img_scaling;
}
