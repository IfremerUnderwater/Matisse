#ifndef CAMCALIB_H
#define CAMCALIB_H

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


class CameraCalib:public QObject
{
    Q_OBJECT
public:
    CameraCalib(vector<string>& imagelist, cv::Size board_size, float square_size, QTextEdit *_text_logger);
    void calibrateMono(MatisseCommon::CameraInfo& _cam_info, double& _reproj_error, bool _display_corners = false);

private:
    void checkAndReorderCorners(vector<cv::Point2f> &_corners);

    vector<string>& m_imagelist;
    cv::Size m_board_size;
    float m_square_size;
    QTextEdit *m_text_logger;

    QDir m_calibration_path;

    void sig_logCalib(QString _message);
};

#endif // CAMCALIB_H
