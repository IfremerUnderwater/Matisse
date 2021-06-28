#include "carto_image.h"

#ifdef _WIN32
#include "gdal_priv.h"
#else
#include "gdal/gdal_priv.h"
#endif
//#include "cpl_conv.h" // for CPLMalloc()

#include <QFileInfo>

static bool gdal_loaded = false;

using namespace cv;
using namespace matisse_image;

namespace nav_tools {

CartoImage::CartoImage() :
    Image()
{
    if(!gdal_loaded)
    {
        GDALAllRegister();
        gdal_loaded = true;
    }

    // in case of north up images, the GT(2) and GT(4) coefficients are zero,
    // and the GT(1) is pixel width, and GT(5) is pixel height.
    // The (GT(0),GT(3)) position is the top left corner of the top left pixel of the raster.
    m_adf_geo_transform[0] = 0;
    m_adf_geo_transform[1] = 1;
    m_adf_geo_transform[2] = 0;
    m_adf_geo_transform[3] = 0;
    m_adf_geo_transform[4] = 0;
    m_adf_geo_transform[5] = 1;
}

bool CartoImage::loadFile(QString _filename)
{
    m_id = 0;
    if (_filename.isEmpty()) {
        m_id = -1;
        return false;
    }

    QFileInfo fileInfo(_filename);
    m_file_name = fileInfo.absoluteFilePath();

    // geotiff data
    GDALDataset  *po_dataset;
    po_dataset = (GDALDataset *) GDALOpen( fileInfo.absoluteFilePath().toStdString().c_str(), GA_ReadOnly );
    if( po_dataset != NULL )
    {
        if( po_dataset->GetGeoTransform( m_adf_geo_transform ) == CE_None )
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
            m_adf_geo_transform[0] = 0;
            m_adf_geo_transform[1] = 1;
            m_adf_geo_transform[2] = 0;
            m_adf_geo_transform[3] = 0;
            m_adf_geo_transform[4] = 0;
            m_adf_geo_transform[5] = 1;
        }
        GDALClose(po_dataset);
    }

    Mat read_image = imread(m_file_name.toStdString().c_str(),cv::IMREAD_COLOR | cv::IMREAD_IGNORE_ORIENTATION);
    m_image_data = new Mat(read_image);

    return true;
}

QRectF CartoImage::getEnvelope()
{
    // rotation
    qreal xmax = m_image_data->cols-1;
    qreal ymax = m_image_data->rows-1;
    qreal left = min(xGeo(0,0), xGeo(xmax, 0));
    left = min(left,  xGeo(0, ymax));
    left = min(left,  xGeo(xmax, ymax));

    qreal right = max(xGeo(0,0), xGeo(xmax, 0));
    right = max(right,  xGeo(0, ymax));
    right = max(right,  xGeo(xmax, ymax));

    qreal top = min(yGeo(0,0), yGeo(xmax, 0));
    top = min(top,  yGeo(0, ymax));
    top = min(top,  yGeo(xmax, ymax));

    qreal bottom = max(yGeo(0,0), yGeo(xmax, 0));
    bottom = max(bottom,  yGeo(0, ymax));
    bottom = max(bottom,  yGeo(xmax, ymax));

    return QRectF(left, top, right - left, bottom - top);
}

// width of image (and not containing envelope)
qreal CartoImage::widthGeo()
{
    qreal xmax = m_image_data->cols-1;
    return hypot( xGeo(xmax, 0) - xGeo(0,0),  yGeo(xmax, 0) - yGeo(0,0));
}

// height of image (and not containing envelope)
qreal CartoImage::heightGeo()
{
    qreal ymax = m_image_data->rows-1;
    return hypot( xGeo(0, ymax) - xGeo(0,0), yGeo(0, ymax) - yGeo(0,0));
}

qreal CartoImage::getRotationAngle()
{
    qreal angle = atan2(m_adf_geo_transform[2], m_adf_geo_transform[1]);
    return angle;
}

} // namespace nav_tools
