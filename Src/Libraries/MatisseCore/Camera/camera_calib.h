#ifndef MATISSE_CAMERA_CALIB_H_
#define MATISSE_CAMERA_CALIB_H_

#include <QObject>
#include <QString>

#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <QTextEdit>
#include <QDir>
#include "camera_info.h"

using namespace std;

namespace matisse {

class CameraCalib:public QObject
{
    Q_OBJECT
public:
    CameraCalib(vector<string>& imagelist, cv::Size board_size, float square_size, QTextEdit *_text_logger);
    void calibrateMono(CameraInfo& _cam_info, double& _reproj_error, bool _display_corners = false);

private:
    void checkAndReorderCorners(vector<cv::Point2f> &_corners);

    vector<string>& m_imagelist;
    cv::Size m_board_size;
    float m_square_size;
    QTextEdit *m_text_logger;

    QDir m_calibration_path;

    void sig_logCalib(QString _message);
};

} // namespace matisse

#endif // MATISSE_CAMERA_CALIB_H_
