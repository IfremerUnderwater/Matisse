﻿#ifndef DrawBlend2DMosaic_H
#define DrawBlend2DMosaic_H


#include "processor.h"
#include "output_data_writer.h"

using namespace matisse;

/**
 * DrawBlend2DMosaic
 * @brief  DrawBlend2DMosaic draw plane mosaic (2D model or 3D model projected on a 2D plane)
 */
class DrawBlend2DMosaic : public OutputDataWriter
{
    Q_OBJECT
    Q_INTERFACES(matisse::OutputDataWriter)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "DrawBlend2DMosaic")
#endif


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
