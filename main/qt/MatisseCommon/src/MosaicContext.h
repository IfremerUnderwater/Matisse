#ifndef MOSAICCONTEXT_H
#define MOSAICCONTEXT_H

#include <opencv2/opencv.hpp>
#include <opencv/cv.h>

using namespace cv;

//structure Matching

struct Matching2Images {

    int indexes[2];

    QVector<Point2d> pointsImage1;

    QVector<Point2d> pointsImage2;

};

typedef QVector<Matching2Images> MatchingData;

/*struct MatchingData {

    QVector<Matching2Images> matching2Images;

};*/

//structure Mosaic

struct MosaicInit {

 QString filename;

 Point3d mosaicOrigin;

 Point2d pixelSize;

 Point2d mosaicSize;

 Mat mosaic_ullr;

 QString utmHemisphere;

 int utmZone;

};

struct Homography {

    QString model;

    QString type;

    Mat matrix;

    Mat covar;

};

struct Pose {

    QString type;

    Mat matrix;

    Mat covar;

};

struct MosaictNode {

    quint32 imageIndex;

    QString dataSetImageName;

    QDateTime imageTimeStamp;

    Homography homo;

    Pose pose;

};

struct MosaicData {

    MosaicInit init;

    QVector<MosaictNode> nodes;

};

Q_DECLARE_METATYPE(MosaicData*);
Q_DECLARE_METATYPE(MatchingData*);

#endif // MOSAICCONTEXT_H
