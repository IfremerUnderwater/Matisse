#ifndef IMAGE_PROCESSING_PREPROCESSING_CORRECTION_H_
#define IMAGE_PROCESSING_PREPROCESSING_CORRECTION_H_

#include <opencv2/core/mat.hpp>
#include <deque>
#include <vector>
#include <QString>
#include <QWidget>

namespace image_processing {

class PreprocessingCorrection
{
public:

	/// <summary>
	/// Constructor
	/// </summary>
	/// <param name="_ws">ws is the sliding window size (number of image used for the computation of the model)</param>
	/// <param name="_parent"> provide a parent widget in case you want to show graphically the progession</param>
	PreprocessingCorrection(int _ws=5, QWidget* _parent = nullptr);

	/// <summary>
	/// Correct images for illumination given a list of files on the disk
	/// </summary>
	/// <param name="_input_img_files">files path list</param>
	/// <param name="_output_path">output folder path</param>
	/// <returns>true on success</returns>
	bool preprocessImageList(QStringList _input_img_files, QString _output_path);

	/// <summary>
	/// Compute median image in the sliding window
	/// </summary>
	/// <returns>true on success</returns>
	bool computeTemporalMedian();

	/// <summary>
	/// Compute pattern model and compensate illumination
	/// </summary>
	/// <param name="_input_image">Input image</param>
	/// <param name="_input_lowres">Input image in low resolution</param>
	/// <param name="_temporal_median_image">Temporal median image</param>
	/// <param name="_output_image">Compensated output image</param>
	/// <returns>true on success</returns>
	bool compensateIllumination(cv::Mat& _input_image, cv::Mat& _input_lowres, cv::Mat& _temporal_median_image, cv::Mat& _output_image);

	/// <summary>
	/// configure the preprocessing steps that are needed
	/// </summary>
	/// <param name="_correct_colors"></param>
	/// <param name="_compensate_illumination"></param>
	/// <param name="_prepro_img_scaling"></param>
	void configureProcessing(const bool _correct_colors = true, const bool _compensate_illumination = true, const double _prepro_img_scaling = 1.0, const double _saturation_threshold = 0.0001, const cv::Mat& _mask_img = cv::Mat());


private:
	int m_ws;
	double m_lowres_comp_scaling;
	double m_prepro_img_scaling;
	bool m_correct_colors;
	bool m_compensate_illumination;
	double m_sat_thres;

	std::vector<cv::Mat> m_bgr_lowres_img;
	cv::Mat m_mask_img;

	cv::Mat m_blue_median_img;
	cv::Mat m_green_median_img;
	cv::Mat m_red_median_img;

	std::deque<cv::Mat> m_blue_stack_images;
	std::deque<cv::Mat> m_green_stack_images;
	std::deque<cv::Mat> m_red_stack_images;

	QWidget* m_graphic_parent;

};

} // namespace image_processing

#endif // IMAGE_PROCESSING_PREPROCESSING_CORRECTION_H_
