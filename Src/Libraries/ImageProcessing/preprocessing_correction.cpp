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
using namespace basic_processing;

namespace image_processing {

PreprocessingCorrection::PreprocessingCorrection(int _ws, QWidget* _parent):m_ws(_ws),
m_lowres_comp_scaling(1.0),
m_prepro_img_scaling(1.0),
m_correct_colors(false),
m_compensate_illumination(false),
m_sat_thres(0.001)
{
	//bgr_lowres_img.resize(3);
	m_graphic_parent = _parent;

	// Make sure that m_ws is odd
	if (m_ws % 2 == 0)
		m_ws += 1;
}

bool PreprocessingCorrection::preprocessImageList(const QStringList& _input_img_files, const QString& _output_path)
{
	// check if we need to reduce image resolution
	cv::Mat first_img = cv::imread(_input_img_files[0].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);

	m_lowres_comp_scaling = sqrt(1e5 / ((double)first_img.cols * first_img.rows)); // process image at maximum 0.1Mpx
	if (m_lowres_comp_scaling > 1.0)
		m_lowres_comp_scaling = 1.0;

	// Setup the mask only once as the same mask is to be used for all images
	cv::Mat lowres_mask;
	if (!m_mask_img.empty()) 
	{
		if (m_lowres_comp_scaling < 1.0)
			cv::resize(m_mask_img, lowres_mask, cv::Size(), m_lowres_comp_scaling, m_lowres_comp_scaling);
		else
			lowres_mask = m_mask_img;
	}

	if (_input_img_files.size() < m_ws && m_compensate_illumination)
	{
		m_compensate_illumination = false;
	}

	const int im_nb = _input_img_files.size();
	const int half_ws = (m_ws-1)/2;

	QProgressDialog prepro_progress(QString("Preprocessing images files"), "Abort processing", 0, 100, m_graphic_parent);
	int k = 1;

	bool exit_required = false;

	// preprocess
	#pragma omp parallel for if (!m_compensate_illumination)
	for (int i = 0; i < im_nb; i++)
	{
		if (exit_required)
			continue;

		if (m_graphic_parent && prepro_progress.wasCanceled())
		{
			exit_required = true;
			continue;
		}

		cv::Mat current_img = cv::imread(_input_img_files[i].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
		
		std::vector<cv::Mat> bgr_lowres_img;

		// in case we need correction we will use the lower res image to speed up
		if (m_correct_colors || m_compensate_illumination)
		{
			cv::Mat reduced_img;

			if (m_lowres_comp_scaling < 1.0) {
				cv::resize(current_img, reduced_img, cv::Size(), m_lowres_comp_scaling, m_lowres_comp_scaling);
				cv::split(reduced_img, bgr_lowres_img);
			}
			else {
				cv::split(current_img, bgr_lowres_img);
			}

		}

		// check if we need to resize image
		if (m_prepro_img_scaling < 1.0)
			cv::resize(current_img, current_img, cv::Size(), m_prepro_img_scaling, m_prepro_img_scaling);

		// correct colors
		std::vector<int> ch1_lim, ch2_lim, ch3_lim;
		if (m_correct_colors)
		{
			// Construct required quantiles vector
			std::vector<double> quantiles;
			quantiles.push_back(m_sat_thres);
			quantiles.push_back(1.0 - m_sat_thres);

			// Get channels saturation limits
			//findImgColorQuantiles(current_img, m_mask_img, quantiles, ch1_lim, ch2_lim, ch3_lim);
			findImgQuantiles(bgr_lowres_img[0], lowres_mask, quantiles, ch1_lim);
			findImgQuantiles(bgr_lowres_img[1], lowres_mask, quantiles, ch2_lim);
			findImgQuantiles(bgr_lowres_img[2], lowres_mask, quantiles, ch3_lim);
		}

		// need illumination compensation
		if (m_compensate_illumination)
		{
			// If first image, just add m_ws images for computing temporal median
			if (i == 0)
			{
				for (int j = 0; j < m_ws; j++)
				{
					cv::Mat temp_img;
					cv::resize(cv::imread(_input_img_files[j].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION), temp_img, cv::Size(), m_lowres_comp_scaling, m_lowres_comp_scaling);

					std::vector<cv::Mat> temp_BGR(3);
					cv::split(temp_img, temp_BGR);

					// stacking images for median
					m_blue_stack_images.push_back(temp_BGR[0]);
					m_green_stack_images.push_back(temp_BGR[1]);
					m_red_stack_images.push_back(temp_BGR[2]);
				}
			}
			else if (i + half_ws >= m_ws && i + half_ws < im_nb)
			{
				cv::Mat temp_img;
				cv::resize(cv::imread(_input_img_files[i + half_ws].toStdString(), cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION), temp_img, cv::Size(), m_lowres_comp_scaling, m_lowres_comp_scaling);

				std::vector<cv::Mat> temp_BGR(3);
				cv::split(temp_img, temp_BGR);

				// stacking images for median
				m_blue_stack_images.pop_front();
				m_blue_stack_images.push_back(temp_BGR[0]);
				m_green_stack_images.pop_front();
				m_green_stack_images.push_back(temp_BGR[1]);
				m_red_stack_images.pop_front();
				m_red_stack_images.push_back(temp_BGR[2]);
			}

			// Compute temporal median
			if (computeTemporalMedian())
			{
				std::vector<cv::Mat> current_bgr(3);
				std::vector<cv::Mat> current_bgr_corr(3);
				cv::split(current_img, current_bgr);

				// compute illum correction
				compensateIllumination(current_bgr[0], bgr_lowres_img[0], m_blue_median_img, current_bgr_corr[0]);
				compensateIllumination(current_bgr[1], bgr_lowres_img[1], m_green_median_img, current_bgr_corr[1]);
				compensateIllumination(current_bgr[2], bgr_lowres_img[2], m_red_median_img, current_bgr_corr[2]);

				cv::merge(current_bgr_corr, current_img);
			}

		} // end need illumination compensation

		// correct colors
		if (m_correct_colors)
		{
			// Strech img according to saturation limit
			stretchColorImg(current_img, ch1_lim, ch2_lim, ch3_lim, current_img, false);
		}

		// write image
		const QFileInfo current_file_info(_input_img_files[i]);
		const QString outfile = _output_path + QDir::separator() + current_file_info.fileName();
		cv::imwrite(outfile.toStdString(), current_img);

		#pragma omp critical
		if (m_graphic_parent)
		{
			prepro_progress.setValue(round(100 * k / im_nb));
			k++;
			QApplication::processEvents();
		}

	}

	if (exit_required)
		return false;

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
	cv::Size images_size = m_blue_stack_images[0].size();
	const size_t stack_size = m_blue_stack_images.size();
	m_blue_median_img = cv::Mat::zeros(images_size, CV_32FC1);
	m_green_median_img = cv::Mat::zeros(images_size, CV_32FC1);
	m_red_median_img = cv::Mat::zeros(images_size, CV_32FC1);

	const double inv_max_val = 1.0 / 255.0;

	#pragma omp parallel for
	for (int w = 0; w < images_size.width; w++)
	{
		for (int h = 0; h < images_size.height; h++)
		{
			std::vector<double> blue_pixel_stack; blue_pixel_stack.resize(stack_size);
			std::vector<double> green_pixel_stack; green_pixel_stack.resize(stack_size);
			std::vector<double> red_pixel_stack; red_pixel_stack.resize(stack_size);

			for (size_t k = 0; k < stack_size; k++)
			{
				blue_pixel_stack[k] = gamma_undo(static_cast<double>(m_blue_stack_images[k].at<uchar>(h, w)) * inv_max_val);
				green_pixel_stack[k] = gamma_undo(static_cast<double>(m_green_stack_images[k].at<uchar>(h, w)) * inv_max_val);
				red_pixel_stack[k] = gamma_undo(static_cast<double>(m_red_stack_images[k].at<uchar>(h, w)) * inv_max_val);
			}

			m_blue_median_img.at<float>(h, w) = static_cast<float>(doubleVectorMedian(blue_pixel_stack));
			m_green_median_img.at<float>(h, w) = static_cast<float>(doubleVectorMedian(green_pixel_stack));
			m_red_median_img.at<float>(h, w) = static_cast<float>(doubleVectorMedian(red_pixel_stack));
		}
	}

	return true;
}

bool PreprocessingCorrection::compensateIllumination(const cv::Mat& _input_image, const cv::Mat& _input_lowres, const cv::Mat& _temporal_median_image, cv::Mat& _output_image)
{
	// This function contains empirical choices about model to correct and thresholds
	// It is not to be understood just adjusted on multiples datasets

	// spatially filter temporal median image
	cv::Mat temporal_spatial_median_image;
	std::vector<double> temporal_spatial_median_vect;
	cv::medianBlur(_temporal_median_image, temporal_spatial_median_image, 5);

	//imshow("median img", _temporal_spatial_median_image);
	//waitKey();

	for (int r = 0; r < temporal_spatial_median_image.rows; r++)
	{
		const float* img_row = temporal_spatial_median_image.ptr<float>(r);
		for (int c = 0; c < temporal_spatial_median_image.cols; c++)
			temporal_spatial_median_vect.push_back(img_row[c]);
	}
  
	const double out_perc = 0.01; // outlier percentage
	const double maximum_corr_factor = 10.0; // if correction is greater we truncate to this value
	
	std::vector<double> quantiles;
	quantiles.push_back(out_perc);
	quantiles.push_back(1.0 - out_perc);

	std::vector<double> channel_limits;
	channel_limits = doubleQuantiles(temporal_spatial_median_vect, quantiles);

	const int nb_med_img_px = temporal_spatial_median_image.cols * temporal_spatial_median_image.rows;
	const int needed_fitting_points_nb = nb_med_img_px > 5000 ? 5000 : nb_med_img_px;

	// Construct x,y,z fitting dataset (x = width, y = height, z=intensity) for paraboloid model
	std::vector<double> x, y, z;
	x.reserve(needed_fitting_points_nb);
	y.reserve(needed_fitting_points_nb);
	z.reserve(needed_fitting_points_nb);

	if (needed_fitting_points_nb == nb_med_img_px)
	{
		for (int r = 0; r < temporal_spatial_median_image.rows; r++)
		{
			const float* img_row = temporal_spatial_median_image.ptr<float>(r);
			for (int c = 0; c < temporal_spatial_median_image.cols; c++)
			{
				const double temp_z = static_cast<double>(img_row[c]);
				if (temp_z > channel_limits[0] && temp_z < channel_limits[1])
				{
					x.push_back(static_cast<double>(c));
					y.push_back(static_cast<double>(r));
					z.push_back(temp_z);
				}
			}
		}
	}
	else 
	{
		int fitting_points_nb = 0;
		std::random_device rd;  //Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_int_distribution<> width_dist(0, temporal_spatial_median_image.cols-1);
		std::uniform_int_distribution<> height_dist(0, temporal_spatial_median_image.rows-1);

		// Set to avoid using two times the same px coords.
		std::set<std::pair<int,int>> checked_px_set;

		while (fitting_points_nb < needed_fitting_points_nb)
		{
			const int w = width_dist(gen);
			const int h = height_dist(gen);

			const std::pair<int,int> px = std::make_pair(w,h);
			if (checked_px_set.count(px))
				continue;
			else
				checked_px_set.insert(px);
			
			const double temp_z = static_cast<double>(temporal_spatial_median_image.at<float>(h, w));

			if (temp_z > channel_limits[0] && temp_z < channel_limits[1])
			{
				x.push_back(static_cast<double>(w));
				y.push_back(static_cast<double>(h));
				z.push_back(temp_z);
				fitting_points_nb++;
			}

			// Safety break in case there is not enough values withib the channel limits 
			// boundaries (in such case leave once all pixels have been checked)
			if ((int)checked_px_set.size() == nb_med_img_px)
				break;
		}
	}

	// Solve paraboloid model
	cv::Mat A = cv::Mat::zeros(cv::Size(10, x.size()), CV_64FC1);
	cv::Mat b = cv::Mat::zeros(cv::Size(1, z.size()), CV_64FC1);
	
	const size_t nb_els = z.size();
	
	#pragma omp parallel for
	for (size_t i = 0; i < nb_els; i++)
	{
		// A = [x. ^ 3, y. ^ 3, x.*y. ^ 2, x. ^ 2. * y, x. ^ 2, y. ^ 2, x.*y, x, y, ones(size(x))];
		// idx  0       1       2          3            4       5       6     7  8  9
		
		const double x1 = x[i];
		const double x2 = x1*x1;
		const double x3 = x2*x1;

		const double y1 = y[i];
		const double y2 = y1*y1;
		const double y3 = y2*y1;

		A.at<double>(i, 0) = x3;
		A.at<double>(i, 1) = y3;
		A.at<double>(i, 2) = x1*y2;
		A.at<double>(i, 3) = x2*y1;
		A.at<double>(i, 4) = x2;
		A.at<double>(i, 5) = y2;
		A.at<double>(i, 6) = x1*y1;
		A.at<double>(i, 7) = x1;
		A.at<double>(i, 8) = y1;
		A.at<double>(i, 9) = 1;

		b.at<double>(i) = z[i];

	}

	cv::Mat alpha;

	cv::solve(A, b, alpha, cv::DECOMP_SVD);

	// Correct image
	cv::Mat _dbl_output_image = cv::Mat(_input_image.size(), CV_64FC1);
	_output_image = cv::Mat(_input_image.size(), _input_image.type());

	// double corr_factor;
	// double lin_input_mean = 0;
	// double lin_corr_mean = 0;
	// double scale_factor = 1.0;

	const double inv_max_val = 1.0 / 255.0;
	const double scaling_factor = m_lowres_comp_scaling / m_prepro_img_scaling;

	// first apply correction without normalization
	#pragma omp parallel for
	for (int r = 0; r < _input_image.rows; r++)
	{
		const uchar* in_img_row = _input_image.ptr<uchar>(r);
		double* out_img_row = _dbl_output_image.ptr<double>(r);

		for (int c = 0; c < _input_image.cols; c++)
		{
			const double temp_x = static_cast<double>(c) * scaling_factor;
			const double temp_y = static_cast<double>(r) * scaling_factor;

			const double temp_x2 = temp_x * temp_x;
			const double temp_y2 = temp_y * temp_y;

			const double temp_x3 = temp_x2 * temp_x;
			const double temp_y3 = temp_y2 * temp_y;

			const double temp_z = alpha.at<double>(0) * temp_x3 + alpha.at<double>(1) * temp_y3 + alpha.at<double>(2) * temp_x * temp_y2
													+ alpha.at<double>(3) * temp_x2 * temp_y + alpha.at<double>(4) * temp_x2 + alpha.at<double>(5) * temp_y2
													+ alpha.at<double>(6) * temp_x * temp_y + alpha.at<double>(7) * temp_x + alpha.at<double>(8) * temp_y 
													+ alpha.at<double>(9);

			double corr_factor = 1.0 / temp_z;

			if (corr_factor > maximum_corr_factor || temp_z < 0.0)
			{
				corr_factor = maximum_corr_factor;
			}

			// correct accounting for gamma factor
			out_img_row[c] = corr_factor*gamma_undo(static_cast<double>(in_img_row[c]) * inv_max_val);
		}
	}

	// compute low res for faster quantile computation
	cv::Mat _dbl_lowres_output_image;
	cv::resize(_dbl_output_image, _dbl_lowres_output_image, _input_lowres.size());

	const int lowres_img_nb_px = _input_lowres.cols * _input_lowres.rows;
	std::vector<double> dbl_lowres_input_vec, dbl_lowres_corr_vec;
	dbl_lowres_input_vec.reserve(lowres_img_nb_px);
	dbl_lowres_corr_vec.reserve(lowres_img_nb_px);

	for (int r = 0; r < _input_lowres.rows; r++)
	{
		const uchar* in_img_row = _input_lowres.ptr<uchar>(r);
		const double* dbl_img_row = _dbl_lowres_output_image.ptr<double>(r);

		for (int c = 0; c < _input_lowres.cols; c++)
		{
			// correct accounting for gamma factor
			dbl_lowres_input_vec.push_back(gamma_undo(static_cast<double>(in_img_row[c]) * inv_max_val));
			dbl_lowres_corr_vec.push_back(dbl_img_row[c]);
		}
	}

	std::vector<double> required_quant, lin_input_quant, lin_corr_quant;

	// We suppose that well illuminated area should match before and after corr so we adjust exposure according to this assumption
	// using 0.8 (so threshold giving the 20% brightest pixels)
	required_quant.push_back(0.8);
	lin_input_quant = doubleQuantiles(dbl_lowres_input_vec, required_quant);

	// add a quantile to check for saturation
	required_quant.push_back(0.999);
	lin_corr_quant = doubleQuantiles(dbl_lowres_corr_vec, required_quant);

	double scale_factor = lin_input_quant[0] / lin_corr_quant[0];

	std::cout << "lin_input_quant=" << lin_input_quant[0] << ", lin_corr_quant=" << lin_corr_quant[0] << std::endl;

	if (scale_factor*lin_corr_quant[1] > 1.0)
	{
		std::cout << "scale factor=" << scale_factor << ", sat =" << lin_corr_quant[1]  << std::endl;
		scale_factor = 1.0 / lin_corr_quant[1];
	}

	// apply scaling factor
	#pragma omp parallel for
	for (int r = 0; r < _input_image.rows; r++)
	{
		const double* dbl_img_row = _dbl_output_image.ptr<double>(r);
		uchar* out_img_row = _output_image.ptr<uchar>(r);

		for (int c = 0; c < _input_image.cols; c++)
		{
			// correct accounting for gamma factor
			out_img_row[c] = cv::saturate_cast<uchar>(255.0 * gamma_do(scale_factor * dbl_img_row[c]));
		}
	}

}

void PreprocessingCorrection::configureProcessing(const bool _correct_colors, const bool _compensate_illumination, const double _prepro_img_scaling, const double _saturation_threshold, const cv::Mat& _mask_img)
{
	m_correct_colors = _correct_colors;
	m_compensate_illumination = _compensate_illumination;
	if (_prepro_img_scaling > 1.0)
		m_prepro_img_scaling = 1.0;
	else
		m_prepro_img_scaling = _prepro_img_scaling;

	if (_saturation_threshold >= 0.0001 && _saturation_threshold <= 0.01)
		m_sat_thres = _saturation_threshold;
	else
		m_sat_thres = 0.001;

	if (!_mask_img.empty())
	{
		_mask_img.copyTo(m_mask_img);
	}
}

} // namespace image_processing
