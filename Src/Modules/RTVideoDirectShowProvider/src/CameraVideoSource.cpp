#include <assert.h>
#include <QTimerEvent>
#include <QDebug>
#include "opencv/cv.h"
#include "opencv/highgui.h"

#include "CameraVideosource.h"



CameraVideoSource::CameraVideoSource(int indCam, int interval):
        _cvInputSource(NULL),
        _cvTypeCamera(CV_CAP_DSHOW),
        _indCam(indCam),
        _interval(interval),
        _visu(false)
      //,       _capturedImage(NULL)

{
    m_CVMR = new CVMR_Capture();    
    previewer = new VideoPreview( NULL, m_CVMR);
}

CameraVideoSource::~CameraVideoSource()
{
    delete m_CVMR;
    delete previewer;
}

bool CameraVideoSource::configure()
{

    return true;
}

bool CameraVideoSource::start()
{
    setOkStatus();

    return true;
}

void CameraVideoSource::stop()
{
    m_CVMR->stop();
}

bool CameraVideoSource::isValid()
{
    return true;
}

NavImage * CameraVideoSource::captureImage(QString dim2Line )
{


   DWORD dwSize = m_CVMR->GrabFrame();
   if(dwSize>0)
      {

      BYTE *pImage = new BYTE[dwSize];
           m_CVMR->GetFrame (&pImage);

    LPBITMAPINFOHEADER  pdib = (LPBITMAPINFOHEADER) m_CVMR->lpCurrImage;


    int height = pdib->biHeight;
    int width = pdib->biWidth;
    int bytesPerLine = 3*pdib->biWidth;

    Mat * matImage = new Mat( height, width, CV_8UC3, const_cast<uchar*>(pImage), bytesPerLine );

    qDebug()<< "Channels avant : " << matImage->channels();

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
    NavImage *image = new NavImage(0,matImage,navInfo);
    return image;
   }
    return NULL;
}


void CameraVideoSource::slot_previewStart()
{
    m_CVMR->Init( 0 , previewer->getVideoWidget(), previewer->getWidth(), previewer->getHeight());
    previewer->show();
}

void CameraVideoSource::slot_previewStop()
{
    previewer->hide();
    stop();
}

