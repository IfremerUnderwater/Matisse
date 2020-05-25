#include "CartoImage.h"

#ifdef _WIN32
#include "gdal_priv.h"
#else
#include "gdal/gdal_priv.h"
#endif
//#include "cpl_conv.h" // for CPLMalloc()

#include <QFileInfo>

static bool gdalLoaded = false;

using namespace cv;

CartoImage::CartoImage() : MatisseCommon::Image()
{
    if(!gdalLoaded)
    {
        GDALAllRegister();
        gdalLoaded = true;
    }

    // in case of north up images, the GT(2) and GT(4) coefficients are zero,
    // and the GT(1) is pixel width, and GT(5) is pixel height.
    // The (GT(0),GT(3)) position is the top left corner of the top left pixel of the raster.
    _adfGeoTransform[0] = 0;
    _adfGeoTransform[1] = 1;
    _adfGeoTransform[2] = 0;
    _adfGeoTransform[3] = 0;
    _adfGeoTransform[4] = 0;
    _adfGeoTransform[5] = 1;
}

bool CartoImage::loadFile(QString filename_p)
{
    _id = 0;
    if (filename_p.isEmpty()) {
        _id = -1;
        return false;
    }

    QFileInfo fileInfo(filename_p);
    _fileName = fileInfo.absoluteFilePath();

    // geotiff data
    GDALDataset  *poDataset;
    poDataset = (GDALDataset *) GDALOpen( fileInfo.absoluteFilePath().toStdString().c_str(), GA_ReadOnly );
    if( poDataset != NULL )
    {
        if( poDataset->GetGeoTransform( _adfGeoTransform ) == CE_None )
        {
            // OK = _adfGeoTransform filled
            //            printf( "Origin = (%.6f,%.6f)\n",
            //                    adfGeoTransform[0], adfGeoTransform[3] );
            //            printf( "Pixel Size = (%.6f,%.6f)\n",
            //                    adfGeoTransform[1], adfGeoTransform[5] );
        }
        else
        {
            // in case of north up images, the GT(2) and GT(4) coefficients are zero,
            // and the GT(1) is pixel width, and GT(5) is pixel height.
            // The (GT(0),GT(3)) position is the top left corner of the top left pixel of the raster.
            _adfGeoTransform[0] = 0;
            _adfGeoTransform[1] = 1;
            _adfGeoTransform[2] = 0;
            _adfGeoTransform[3] = 0;
            _adfGeoTransform[4] = 0;
            _adfGeoTransform[5] = 1;
        }
        GDALClose(poDataset);
    }

    Mat readImage = imread(_fileName.toStdString().c_str(),cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
    _imageData = new Mat(readImage);

    return true;
}

QRectF CartoImage::getEnvelope()
{
    // rotation
    qreal XMAX = _imageData->cols-1;
    qreal YMAX = _imageData->rows-1;
    qreal left = min(xGeo(0,0), xGeo(XMAX, 0));
    left = min(left,  xGeo(0, YMAX));
    left = min(left,  xGeo(XMAX, YMAX));

    qreal right = max(xGeo(0,0), xGeo(XMAX, 0));
    right = max(right,  xGeo(0, YMAX));
    right = max(right,  xGeo(XMAX, YMAX));

    qreal top = min(yGeo(0,0), yGeo(XMAX, 0));
    top = min(top,  yGeo(0, YMAX));
    top = min(top,  yGeo(XMAX, YMAX));

    qreal bottom = max(yGeo(0,0), yGeo(XMAX, 0));
    bottom = max(bottom,  yGeo(0, YMAX));
    bottom = max(bottom,  yGeo(XMAX, YMAX));

    return QRectF(left, top, right - left, bottom - top);
}

// width of image (and not containing envelope)
qreal CartoImage::widthGeo()
{
    qreal XMAX = _imageData->cols-1;
    return hypot( xGeo(XMAX, 0) - xGeo(0,0),  yGeo(XMAX, 0) - yGeo(0,0));
}

// height of image (and not containing envelope)
qreal CartoImage::heightGeo()
{
    qreal YMAX = _imageData->rows-1;
    return hypot( xGeo(0, YMAX) - xGeo(0,0), yGeo(0, YMAX) - yGeo(0,0));
}

qreal CartoImage::getRotationAngle()
{
    qreal angle = atan2(_adfGeoTransform[2], _adfGeoTransform[1]);
    return angle;
}
