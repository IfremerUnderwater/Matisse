#ifndef DrawBlend2DMosaic_H
#define DrawBlend2DMosaic_H


#include "Processor.h"
#include "RasterProvider.h"

using namespace MatisseCommon;

/**
 * DrawBlend2DMosaic
 * @brief  DrawBlend2DMosaic draw plane mosaic (2D model or 3D model projected on a 2D plane)
 */
class DrawBlend2DMosaic : public RasterProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::RasterProvider)

public:
    DrawBlend2DMosaic();
    ~DrawBlend2DMosaic();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, Image &image);
    virtual QList<QFileInfo> rastersInfo();

private:
    QList<QFileInfo> _rastersInfo;
    QString _outputDirnameStr;
};

#endif // DrawBlend2DMosaic_H
