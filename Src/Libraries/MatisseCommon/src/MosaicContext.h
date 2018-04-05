#ifndef MOSAICCONTEXT_H
#define MOSAICCONTEXT_H

#include <opencv2/opencv.hpp>
#include <opencv/cv.h>
#include <QVector>
#include <QString>
#include <QDateTime>
#include <QVariant>

class ProjectiveCamera;
class MosaicDescriptor;
//using namespace cv;

//structure Matching

struct Matching2Images {

    int indexes[2];

    QVector<cv::Point2d> pointsImage1;
    QVector<cv::Point2d> pointsImage2;

};

typedef QVector<Matching2Images> MatchingData;


struct MosaicInit {

 QString filename;
 cv::Point3d mosaicOrigin;
 cv::Point2d pixelSize;
 cv::Point2d mosaicSize;
 cv::Mat mosaic_ullr;
 QString utmHemisphere;

 int utmZone;

};

struct Homography {

    QString model;
    QString type;
    cv::Mat matrix;
    cv::Mat covar;

};

struct Pose {

    QString type;
    cv::Mat matrix;
    cv::Mat covar;

};

struct MosaicNode {

    quint32 imageIndex;
    QString dataSetImageName;
    QDateTime imageTimeStamp;
    Homography homo;
    Pose pose;

};

struct MosaicData {

    MosaicInit init;

    QVector<MosaicNode> nodes;

};

Q_DECLARE_METATYPE(MosaicData*)
Q_DECLARE_METATYPE(MatchingData*)
Q_DECLARE_METATYPE(QVector<ProjectiveCamera*>*)
Q_DECLARE_METATYPE(MosaicDescriptor*)
#endif // MOSAICCONTEXT_H
