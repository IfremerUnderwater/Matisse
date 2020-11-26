#include "preprocessing_correction.h"
#include "imageprocessing.h"
#include "stdvectoperations.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <QFileInfo>
#include <QDir>
#include <QProgressDialog>
#include <QApplication>
#include <random>

#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

PreprocessingCorrection::PreprocessingCorrection(int _ws, QWidget* _parent):m_ws(_ws),
m_lowres_comp_scaling(1.0),
m_prepro_img_scaling(1.0),
m_correct_colors(false),
m_compensate_illumination(false)
{
	//m_bgr_lowres_img.resize(3);
	m_graphic_parent = _parent;
}

bool PreprocessingCorrection::preprocessImageList(QStringList _input_img_files, QString _output_path)
{
	// check if we need to reduce image resolution
	cv::Mat first_img = cv::imread(_input_img_files[0].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
	m_lowres_comp_scaling = sqrt(1e5 / ((double)first_img.cols * first_img.rows)); // process image at maximum 0.1Mpx
	if (m_lowres_comp_scaling > 1.0)
		m_lowres_comp_scaling = 1.0;

	if (_input_img_files.size() < m_ws)
		return false;

	int im_nb = _input_img_files.size();
	int half_ws = (m_ws-1)/2;

	QProgressDialog prepro_progress(QString("Preprocessing images files"), "Abort processing", 0, 100, m_graphic_parent);
	int j = 1;

	// preprocess
	for (int i = 0; i < im_nb; i++)
	{
		if (m_graphic_parent && prepro_progress.wasCanceled())
			return false;

		Mat current_img = imread(_input_img_files[i].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);

		// in case we need correction we will use the lower res image to speed up
		if (m_correct_colors || m_compensate_illumination)
		{
			Mat reduced_img;

			if (m_lowres_comp_scaling < 1.0) {
				resize(current_img, reduced_img, Size(), m_lowres_comp_scaling, m_lowres_comp_scaling);
				split(reduced_img, m_bgr_lowres_img);
			}
			else {
				split(current_img, m_bgr_lowres_img);
			}

		}

		// check if we need to resize image
		if (m_prepro_img_scaling < 1.0)
			resize(current_img, current_img, Size(), m_prepro_img_scaling, m_prepro_img_scaling);

		// correct colors
		vector<int> ch1_lim, ch2_lim, ch3_lim;
		cv::Mat empty_mask;
		if (m_correct_colors)
		{
			// Construct required quantiles vector
			vector<double> quantiles;
			quantiles.push_back(0.001);
			quantiles.push_back(1 - 0.001);

			// Get channels saturation limits
			//findImgColorQuantiles(current_img, empty_mask, quantiles, ch1_lim, ch2_lim, ch3_lim);
			findImgQuantiles(m_bgr_lowres_img[0], empty_mask, quantiles, ch1_lim);
			findImgQuantiles(m_bgr_lowres_img[1], empty_mask, quantiles, ch2_lim);
			findImgQuantiles(m_bgr_lowres_img[2], empty_mask, quantiles, ch3_lim);
		}

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
						resize(imread(_input_img_files[j].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION), temp_img, Size(), m_lowres_comp_scaling, m_lowres_comp_scaling);

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
				resize(imread(_input_img_files[i + half_ws].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION), temp_img, Size(), m_lowres_comp_scaling, m_lowres_comp_scaling);

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
			compensateIllumination(current_BRG[0], m_bgr_lowres_img[0], m_blue_median_img, current_BRG_corr[0]);
			compensateIllumination(current_BRG[1], m_bgr_lowres_img[1], m_green_median_img, current_BRG_corr[1]);
			compensateIllumination(current_BRG[2], m_bgr_lowres_img[2], m_red_median_img, current_BRG_corr[2]);

			merge(current_BRG_corr, current_img);

		} // end need illumination compensation

				// correct colors
		if (m_correct_colors)
		{

			// Strech img according to saturation limit
			stretchColorImg(current_img, empty_mask, ch1_lim, ch2_lim, ch3_lim, current_img, false);
		}

		// write image
		QFileInfo current_file_info(_input_img_files[i]);
		QString outfile = _output_path + QDir::separator() + current_file_info.fileName();
		cv::imwrite(outfile.toStdString(), current_img);

		if (m_graphic_parent)
		{
			prepro_progress.setValue(round(100 * j / im_nb));
			j++;
			QApplication::processEvents();
		}

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

	vector<double> blue_pixel_stack; blue_pixel_stack.resize(stack_size);
	vector<double> green_pixel_stack; green_pixel_stack.resize(stack_size);
	vector<double> red_pixel_stack; red_pixel_stack.resize(stack_size);

	#pragma omp parallel for
	for (int w = 0; w < images_size.width; w++)
	{
		for (int h = 0; h < images_size.height; h++)
		{
			for (int k = 0; k < m_blue_stack_images.size(); k++)
			{
				blue_pixel_stack[k] = gamma_undo(static_cast<double>(m_blue_stack_images[k].at<uchar>(h, w)) / 255.0);
				green_pixel_stack[k] = gamma_undo(static_cast<double>(m_green_stack_images[k].at<uchar>(h, w)) / 255.0);
				red_pixel_stack[k] = gamma_undo(static_cast<double>(m_red_stack_images[k].at<uchar>(h, w)) / 255.0);
			}
			double tempMedian;

			tempMedian = doubleVectorMedian(blue_pixel_stack);
			m_blue_median_img.at<float>(h, w) = static_cast<float>(tempMedian);

			tempMedian = doubleVectorMedian(green_pixel_stack);
			m_green_median_img.at<float>(h, w) = static_cast<float>(tempMedian);

			tempMedian = doubleVectorMedian(red_pixel_stack);
			m_red_median_img.at<float>(h, w) = static_cast<float>(tempMedian);

		}
	}

	return true;
}

bool PreprocessingCorrection::compensateIllumination(Mat& _input_image, Mat& _input_lowres, Mat& _temporal_median_image, Mat& _output_image)
{
	// This function contains empirical choices about model to correct and thresholds
	// It is not to be understood just adjusted on multiples datasets

	// spatially filter temporal median image
	Mat _temporal_spatial_median_image;
	std::vector<double> _temporal_spatial_median_vect;
	medianBlur(_temporal_median_image, _temporal_spatial_median_image, 5);

	//imshow("median img", _temporal_spatial_median_image);
	//waitKey();

	double out_perc = 0.01; // outlier percentage
	double maximum_corr_factor = 10.0; // if correction is greater we truncate to this value
	vector<double> channel_limits;
	vector<double> quantiles;
	quantiles.push_back(out_perc);
	quantiles.push_back(1 - out_perc);

	for (int w = 0; w < _temporal_spatial_median_image.cols; w++)
	{
		for (int h = 0; h < _temporal_spatial_median_image.rows; h++)
		{
			_temporal_spatial_median_vect.push_back(_temporal_spatial_median_image.at<float>(h, w));
		}
	}
	channel_limits = doubleQuantiles(_temporal_spatial_median_vect, quantiles);

	// Construct x,y,z fitting dataset (x = width, y = height, z=intensity) for paraboloid model
	vector<double> x, y, z;
	double temp_x, temp_y, temp_z;
	int needed_fitting_points_nb = 5000;
	
	int fitting_points_nb = 0;
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> width_dist(0, _temporal_spatial_median_image.cols-1);
	std::uniform_int_distribution<> height_dist(0, _temporal_spatial_median_image.rows-1);

	while (fitting_points_nb < needed_fitting_points_nb)
	{
		int w = width_dist(gen);
		int h = height_dist(gen);
		temp_x = static_cast<double>( w );
		temp_y = static_cast<double>( h );
		temp_z = _temporal_spatial_median_image.at<float>(h, w);

		if (temp_z > channel_limits[0] && temp_z < channel_limits[1])
		{
			x.push_back(w);
			y.push_back(h);
			z.push_back(temp_z);
			fitting_points_nb++;
		}
	}

	// Solve paraboloid model
	Mat A = cv::Mat::zeros(cv::Size(10, x.size()), CV_64FC1);
	Mat b = cv::Mat::zeros(cv::Size(1, z.size()), CV_64FC1);
	
	for (int i = 0; i < z.size(); i++)
	{
		// A = [x. ^ 3, y. ^ 3, x.*y. ^ 2, x. ^ 2. * y, x. ^ 2, y. ^ 2, x.*y, x, y, ones(size(x))];
		// idx  0       1       2          3            4       5       6     7  8  9
		A.at<double>(i, 0) = pow(x[i], 3);
		A.at<double>(i, 1) = pow(y[i], 3);
		A.at<double>(i, 2) = x[i]*pow(y[i], 2);
		A.at<double>(i, 3) = pow(x[i],2)*y[i];
		A.at<double>(i, 4) = pow(x[i], 2);
		A.at<double>(i, 5) = pow(y[i], 2);
		A.at<double>(i, 6) = x[i]*y[i];
		A.at<double>(i, 7) = x[i];
		A.at<double>(i, 8) = y[i];
		A.at<double>(i, 9) = 1;

		b.at<double>(i) = z[i];

	}

	Mat alpha;

	solve(A, b, alpha, DECOMP_SVD);

	// Correct image
	vector<double> dbl_lowres_input_vec, dbl_lowres_corr_vec;
	Mat _dbl_output_image = Mat(_input_image.size(), CV_64FC1);
	Mat _dbl_lowres_output_image;
	_output_image = _input_image;

	double corr_factor;
	double lin_input_mean = 0;
	double lin_corr_mean = 0;
	double scale_factor = 1.0;

	// first apply correction without normalization
	#pragma omp parallel for
	for (int w = 0; w < _input_image.cols; w++)
	{
		for (int h = 0; h < _input_image.rows; h++)
		{
			temp_x = static_cast<double>(w) * m_lowres_comp_scaling / m_prepro_img_scaling;
			temp_y = static_cast<double>(h) * m_lowres_comp_scaling / m_prepro_img_scaling;
			temp_z = alpha.at<double>(0) * pow(temp_x, 3) + alpha.at<double>(1) * pow(temp_y, 3) + alpha.at<double>(2) * temp_x * pow(temp_y, 2)
				+ alpha.at<double>(3) * pow(temp_x, 2) * temp_y + alpha.at<double>(4) * pow(temp_x, 2) + alpha.at<double>(5) * pow(temp_y, 2)
				+ alpha.at<double>(6) * temp_x * temp_y + alpha.at<double>(7) * temp_x + alpha.at<double>(8) * temp_y + alpha.at<double>(9);

			corr_factor = 1.0 / (temp_z);

			if (corr_factor > maximum_corr_factor || temp_z < 0)
			{
				corr_factor = maximum_corr_factor;
			}

			// correct accounting for gamma factor
			//_output_image.at<uchar>(h, w) = static_cast<uchar>(255 * gamma_do(corr_factor * gamma_undo(static_cast<double>(_input_image.at<uchar>(h, w)) / 255.0)));
			_dbl_output_image.at<double>(h, w) = corr_factor*gamma_undo(static_cast<double>(_input_image.at<uchar>(h, w))/255.0);

		}
	}

	// compute low res for faster quantile computation
	resize(_dbl_output_image, _dbl_lowres_output_image, _input_lowres.size());

	for (int w = 0; w < _input_lowres.cols; w++)
	{
		for (int h = 0; h < _input_lowres.rows; h++)
		{
			// correct accounting for gamma factor
			double dbl_input = gamma_undo(static_cast<double>(_input_lowres.at<uchar>(h, w)) / 255.0);
			dbl_lowres_input_vec.push_back(dbl_input);

			dbl_lowres_corr_vec.push_back(_dbl_lowres_output_image.at<double>(h, w));

		}
	}

	vector<double> required_quant, lin_input_quant,lin_corr_quant;

	// We suppose that well illuminated area should match before and after corr so we adjust exposure according to this assumption
	// using 0.8 (so threshold giving the 20% brightest pixels)
	required_quant.push_back(0.8);
	lin_input_quant = doubleQuantiles(dbl_lowres_input_vec, required_quant);

	// add a quantile to check for saturation
	required_quant.push_back(0.999);
	lin_corr_quant = doubleQuantiles(dbl_lowres_corr_vec, required_quant);

	scale_factor = lin_input_quant[0] / lin_corr_quant[0];

	cout << "lin_input_quant=" << lin_input_quant[0] << ", lin_corr_quant=" << lin_corr_quant[0] <<endl;

	if (scale_factor*lin_corr_quant[1] > 1)
	{
		cout << "scale factor=" << scale_factor << ", sat =" << lin_corr_quant[1]  <<endl;
		scale_factor = 1.0 /lin_corr_quant[1];
	}

	// apply scaling factor
	#pragma omp parallel for
	for (int w = 0; w < _input_image.cols; w++)
	{
		for (int h = 0; h < _input_image.rows; h++)
		{
			// correct accounting for gamma factor
			_output_image.at<uchar>(h, w) = static_cast<uchar>(255 * gamma_do(scale_factor * _dbl_output_image.at<double>(h, w)) );
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
