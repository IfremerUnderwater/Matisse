#include "camera_calib.h"
#include <QFileInfo>


using namespace cv;

namespace matisse {

CameraCalib::CameraCalib(vector<string>& _imagelist, Size _board_size, float _square_size, QTextEdit* _text_logger = nullptr) :
    m_imagelist(_imagelist),
    m_board_size(_board_size),
    m_square_size(_square_size),
    m_text_logger(_text_logger)
{
    QString first_file_string = QString(_imagelist[0].c_str());
	QFileInfo first_file(first_file_string);
	m_calibration_path = first_file.dir();
}

void
CameraCalib::calibrateMono(CameraInfo& _cam_info, double& _reproj_error, bool _display_corners)
{

    const int max_scale = 2;
	// ARRAY AND VECTOR STORAGE:

    vector<vector<Point2f> > image_points;
    vector<vector<Point3f> > object_points;
    Size image_size;

	int i, j, k, nimages = (int)m_imagelist.size();

    image_points.resize(nimages);
    vector<string> good_image_list;

	for (i = j = 0; i < nimages; i++)
	{

		const string& filename = m_imagelist[i];
		Mat img = imread(filename, 0);
		if (img.empty())
			break;

        if (image_size == Size())
            image_size = img.size();

		// find chessboard corners
		bool found = false;
        vector<Point2f>& corners = image_points[j];
        for (int scale = 1; scale <= max_scale; scale++)
		{
			Mat timg;
			if (scale == 1)
				timg = img;
			else
				resize(img, timg, Size(), scale, scale);
			found = findChessboardCorners(timg, m_board_size, corners,
				CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);
			if (found)
			{
				if (scale > 1)
				{
                    Mat corners_mat(corners);
                    corners_mat *= 1. / scale;
				}
				break;
			}
		}

		if (!found)
		{
			std::cout << "no corner found !!\n";
			continue;
		}
		// refine subpixel corners position
		cornerSubPix(img, corners, Size(11, 11), Size(-1, -1),
			TermCriteria(TermCriteria::COUNT + TermCriteria::EPS,
				30, 0.01));

		// check & reorder corners
		checkAndReorderCorners(corners);

		// display corners
		if (_display_corners)
		{
			//cout << filename << endl;
			Mat cimg, cimg1;
			cvtColor(img, cimg, COLOR_GRAY2BGR);
			drawChessboardCorners(cimg, m_board_size, corners, found);
			double sf = 640. / MAX(img.rows, img.cols);
			resize(cimg, cimg1, Size(), sf, sf);

			cv::imshow(tr("Image with detected corners").toStdString(), cimg1);
			cv::waitKey(100);
		}


        good_image_list.push_back(m_imagelist[i]);
		j++;
	}
	sig_logCalib(QString(tr("%1 image(s) have been successfully detected.\n")).arg(j));
	nimages = j;
	if (nimages < 2)
	{
		sig_logCalib(tr("Error: too little images to run the calibration\n"));
		return;
	}

    image_points.resize(nimages);
    object_points.resize(nimages);

	// Create 3D points in object pattern frame
	for (i = 0; i < nimages; i++)
	{
		for (j = 0; j < m_board_size.height; j++)
			for (k = 0; k < m_board_size.width; k++)
                object_points[i].push_back(Point3f(k * m_square_size, j * m_square_size, 0));
	}

	sig_logCalib(tr("Running monocular calibration ...\n"));

    Mat camera_matrix, dist_coeffs;

	// Calibrate camera
	/*cameraMatrix = initCameraMatrix2D(objectPoints,imagePoints,imageSize,0);*/
    camera_matrix = Mat::eye(3, 3, CV_64F);
    dist_coeffs = Mat::zeros(5, 1, CV_64F);
	vector<Mat> rvecs0, tvecs0;

	int flags=0;
	double rms_reproj;

	if (_cam_info.distortionModel() == 3)
	{
		Mat dist_temp = Mat::zeros(4, 1, CV_64F); // opencv fisheye wants a size 4 vector even if last element is 0
        rms_reproj = fisheye::calibrate(object_points, image_points, image_size, camera_matrix,
			dist_temp, rvecs0, tvecs0, fisheye::CALIB_RECOMPUTE_EXTRINSIC | fisheye::CALIB_FIX_SKEW);

		for (int i = 0; i < 4; i++)
            dist_coeffs.at<double>(i, 0) = dist_temp.at<double>(i, 0);

	}
	else {

		switch (_cam_info.distortionModel())
		{
		case 0:
			flags = CALIB_FIX_ASPECT_RATIO | CALIB_FIX_K2 | CALIB_FIX_K3 | CALIB_FIX_K4 | CALIB_FIX_K5;
			break;
		case 1:
			flags = CALIB_FIX_ASPECT_RATIO | CALIB_FIX_K4 | CALIB_FIX_K5;
			break;
		case 2:
			flags = CALIB_FIX_ASPECT_RATIO;
			break;
		}

        rms_reproj = calibrateCamera(object_points, image_points, image_size, camera_matrix,
            dist_coeffs, rvecs0, tvecs0, flags);

	}

    bool ok = checkRange(camera_matrix) && checkRange(dist_coeffs);
	if (!ok)
	{
		sig_logCalib("Error camera not well calibrated");
		_reproj_error = -1;
		return;
	}

    _cam_info.setDistortionCoeff(dist_coeffs);
    _cam_info.setK(camera_matrix);
    _cam_info.setFullSensorSize(image_size.width, image_size.height);

	sig_logCalib(QString(tr("Camera calibration reprojection error = %1\n")).arg(rms_reproj));

	_reproj_error = rms_reproj;

}

void CameraCalib::checkAndReorderCorners(vector<Point2f>& _corners)
{
	// By convention we choose that first point is more left than last point
	if (_corners[0].x > _corners[_corners.size() - 1].x)
	{
		vector<Point2f> reversed_corners;
		int nb_corners = _corners.size();
		for (int i = 0; i < nb_corners; i++)
		{
			reversed_corners.push_back(_corners[nb_corners - i - 1]);
		}
		_corners.clear();
		_corners = reversed_corners;
	}
}

void CameraCalib::sig_logCalib(QString _message)
{
	if (m_text_logger)
		m_text_logger->append(_message);
}

} // namespace matisse
