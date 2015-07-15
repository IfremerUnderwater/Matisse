#ifndef RASTERGEOREFERENCER_H
#define RASTERGEOREFERENCER_H


#include <gdal/cpl_vsi.h>
#include <gdal/cpl_conv.h>
#include <gdal/cpl_string.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogr_spatialref.h>
#include <gdal/vrtdataset.h>
#include <opencv2/opencv.hpp>
#include <QString>
#include <QStringList>

using namespace cv;

namespace MatisseCommon {

/*! This class is an encapsulation of gdal_translate code to create GeoTiff from a memory raster*/

class RasterGeoreferencer
{
public:
    RasterGeoreferencer();

    ///
    /// \brief WriteGeoFile write a georeferenced file using memory raster
    /// \param raster opencv image of the raster data
    /// \param rasterMask opencv image of the raster mask (0 not to be seen, 255 valid area)
    /// \param outputFile path and name of the output geofile
    /// \param cmdLineOptions gdal options as would be provided to gdal_translate function (see gdal doc for exhaustive list)
    /// \return 0 on success
    ///
    int WriteGeoFile(Mat &raster, Mat &rasterMask, QString outputFile, QString cmdLineOptions);

private:

    GDALDataset         *poDataset;
    GDALDatasetH	    hDataset, hOutDS;
    int			        nRasterXSize, nRasterYSize;
    const char		    *pszDest, *pszFormat ;
    int                 bFormatExplicitelySet;
    GDALDriverH		    hDriver;
    GDALDriver          *inputDriver;
    int			        *panBandList; /* negative value of panBandList[i] means mask band of ABS(panBandList[i]) */
    int                 nBandCount, bDefBands;
    double		        *adfGeoTransform;
    GDALDataType	    eOutputType;
    int			        nOXSize, nOYSize;
    char		        *pszOXSize, *pszOYSize;
    char                **papszCreateOptions;
    int                 *anSrcWin, bStrict;
    const char          *pszProjection;
    int                 bScale, bHaveScaleSrc, bUnscale;
    double	            dfScaleSrcMin, dfScaleSrcMax;
    double              dfScaleDstMin, dfScaleDstMax;
    double              dfULX, dfULY, dfLRX, dfLRY;
    char                **papszMetadataOptions;
    char                *pszOutputSRS;
    int                 bQuiet, bGotBounds;
    GDALProgressFunc    pfnProgress;
    int                 nGCPCount;
    GDAL_GCP            *pasGCPs;
    int                 iSrcFileArg, iDstFileArg;
    int                 bCopySubDatasets;
    double              *adfULLR;
    int                 bSetNoData;
    int                 bUnsetNoData;
    double		        dfNoDataReal;
    int                 nRGBExpand;
    int                 bParsedMaskArgument;
    int                 eMaskMode;
    int                 nMaskBand;
    int                 bStats, bApproxStats;
    int                 bErrorOnPartiallyOutside;
    int                 bErrorOnCompletelyOutside;

};

}

#endif // RASTERGEOREFERENCER_H
