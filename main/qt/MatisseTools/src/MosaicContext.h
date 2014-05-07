#ifndef MOSAICCONTEXT_H
#define MOSAICCONTEXT_H

#include <opencv2/opencv.hpp>

//structure Matching

struct Matching2Images {

    int indexes[2];

    QVector<Point2f> pointsImage1;

    QVector<Point2f> pointsImage2;

};

typedef QVector<Matching2Images> MatchingData;

/*struct MatchingData {

    QVector<Matching2Images> matching2Images;

};*/

//structure Mosaic

struct MosaicInit {

 QString filename;

 Point3f mosaicOrigin;

 Point2f pixelSize;

 Point2f mosaicSize;

 Mat mosaic_ullr;

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

#endif // MOSAICCONTEXT_H
