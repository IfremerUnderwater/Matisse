#include <assert.h>
#include "CameraVideosource.h"
CameraVideoSource::CameraVideoSource(int indCam):
        _cvInputSource(NULL),
        _cvTypeCamera(CV_CAP_DSHOW),
        _indCam(indCam),
        _visu(false)
{


}

bool CameraVideoSource::configure()
{
    bool ret = true;
    int cvValCam = _cvTypeCamera + _indCam;
    _cvInputSource = cvCreateCameraCapture(cvValCam);
    if( !_cvInputSource )
         ret = false;
    cvReleaseCapture(&_cvInputSource );
    return ret;
}

void CameraVideoSource::start()
{
    int cvValCam = _cvTypeCamera + _indCam;
    _cvInputSource = cvCreateCameraCapture(cvValCam);
    _idTimerVideo = startTimer(200);  // 0.1-second timer
}

void CameraVideoSource::stop()
{
    killTimer(_idTimerVideo);
    cvReleaseCapture(&_cvInputSource );
}

bool CameraVideoSource::isValid()
{
    return true;
}

NavImage * CameraVideoSource::captureImage(QString dim2Line )
{
    Dim2 dimCopy(dim2Line);
    NavInfo navInfo;
    navInfo.setInfo(dimCopy.diveNumber(),
                    dimCopy.dateTime(),
                    dimCopy.longitude(),
                    dimCopy.latitude(),
                    dimCopy.depth(),
                    dimCopy.altitude(),
                    dimCopy.yaw(),
                    dimCopy.roll(),
                    dimCopy.pitch(),
                    dimCopy.vx(),
                    dimCopy.vy(),
                    dimCopy.vz());

    IplImage *capture = cvQueryFrame(_cvInputSource);
    Mat * mat = new Mat( capture );
    NavImage *image = new NavImage(0,mat,navInfo);
    return image;
}

void CameraVideoSource::timerEvent(QTimerEvent *evt) {

    IplImage *image=cvQueryFrame(_cvInputSource);
    if( _visu )
    {
        IplImage imCopy(*image);
        cvShowImage("Preview",(CvArr*)&imCopy);
    }


}
