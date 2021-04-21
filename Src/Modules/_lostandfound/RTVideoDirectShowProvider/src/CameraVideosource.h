#ifndef CAMERAVIDEOSOURCE_H
#define CAMERAVIDEOSOURCE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <atlbase.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <QStringList>


#include "ImageSet.h"
#include "NavInfo.h"
#include "NavImage.h"
#include "Dim2.h"

#include "videopreview.h"
#include "VMR_Capture.h"

//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

using namespace MatisseCommon;

class CameraVideoSource : public QObject
{
    Q_OBJECT


public:
    CameraVideoSource(int indCam , int interval);
    virtual ~CameraVideoSource();
    bool configure();
    bool start();
    void stop();
    bool isValid();
    NavImage* captureImage( const QString dim );

private:
    CvCapture * _cvInputSource;
    int _cvTypeCamera;
    int _indCam;
    int _interval;
    int _idTimerVideo;
    int _idTimerCaptur;
    bool _visu;

    VideoPreview * previewer;
    CVMR_Capture * m_CVMR;

public slots:
    void slot_previewStart();
    void slot_previewStop();

signals:
    void imageReady();

};

#endif // CAMERAVIDEOSOURCE_H
