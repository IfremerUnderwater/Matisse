#ifndef RTSurveyPlotter_H
#define RTSurveyPlotter_H


#include "RasterProvider.h"
#include "PictureFileSet.h"
#include "ImageSet.h"
#include "FileImage.h"

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

signals:
    
public slots:
    
};

#endif // RTSurveyPlotter_H
