#ifndef CAMERAVIDEOSOURCE_H
#define CAMERAVIDEOSOURCE_H
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "VideoSource.h"
#include "ImageSet.h"
#include "NavInfo.h"
#include "NavImage.h"
#include "Dim2.h"

using namespace MatisseCommon;

class CameraVideoSource : public VideoSource
{
public:
    CameraVideoSource( int indCam );
    bool configure();
    void start();
    void stop();
    bool isValid();

private:
    CvCapture * _cvInputSource;
    int _cvTypeCamera;
    int _indCam;
    int _idTimerVideo;
    bool _visu;

    void timerEvent(QTimerEvent *evt);

public:
    NavImage* captureImage( const QString dim );
};

#endif // CAMERAVIDEOSOURCE_H
