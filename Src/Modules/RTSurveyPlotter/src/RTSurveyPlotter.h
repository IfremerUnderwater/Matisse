#ifndef RTSurveyPlotter_H
#define RTSurveyPlotter_H


#include "RasterProvider.h"
#include "PictureFileSet.h"
#include "ImageSet.h"
#include "FileImage.h"
#include "MosaicDescriptor.h"

using namespace MatisseCommon;

class RTSurveyPlotter : public RasterProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::RasterProvider)
public:
    explicit RTSurveyPlotter(QObject *parent = 0);
    virtual ~RTSurveyPlotter();

    virtual void onNewImage(quint32 port, Image &image);
    virtual void onFlush(quint32 port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual QList<QFileInfo> rastersInfo();

private:
    QList<QFileInfo> _rastersInfo;
    QString _outputDirnameStr;
    QString _outputFilename;
    bool _doRealTimeMosaicking;

    QVector<ProjectiveCamera*> *_pCams;
    QList<Image *> _imageList;
    MosaicDescriptor *_pMosaicD;
    QString _utmZone;
    cv::Mat _K;
    cv::Mat _V_T_C;
    cv::Mat _V_R_C;
    double _scaleFactor;
    int _sensorFullWidth,_sensorFullHeight;

signals:
    
public slots:
    
};

#endif // RTSurveyPlotter_H
