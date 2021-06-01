#include "raster_georeferencer.h"
#include <QDebug>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <string.h>
#include <stdio.h>

using namespace cv;
using namespace std;

namespace nav_tools {

static int ArgIsNumeric( const char * );
static void AttachMetadata( GDALDatasetH, char ** );
static void CopyBandInfo( GDALRasterBand * poSrcBand, GDALRasterBand * poDstBand,
                          int bCanCopyStatsMetadata, int bCopyScale, int bCopyNoData );
static int bSubCall = FALSE;

/*  ******************************************************************* */
/*           Kept usage from old command line gdal_translate            */
/* ******************************************************************** */

static void Usage(const char* pszErrorMsg = NULL, int bShort = TRUE)

{
    int	iDr;

    printf( "Usage: gdal_translate [--help-general] [--long-usage]\n"
            "       [-ot {Byte/Int16/UInt16/UInt32/Int32/Float32/Float64/\n"
            "             CInt16/CInt32/CFloat32/CFloat64}] [-strict]\n"
            "       [-of format] [-b band] [-mask band] [-expand {gray|rgb|rgba}]\n"
            "       [-outsize xsize[%%] ysize[%%]]\n"
            "       [-unscale] [-scale [src_min src_max [dst_min dst_max]]]\n"
            "       [-srcwin xoff yoff xsize ysize] [-projwin ulx uly lrx lry] [-epo] [-eco]\n"
            "       [-a_srs srs_def] [-a_ullr ulx uly lrx lry] [-a_nodata value]\n"
            "       [-gcp pixel line easting northing [elevation]]*\n"
            "       [-mo \"META-TAG=VALUE\"]* [-q] [-sds]\n"
            "       [-co \"NAME=VALUE\"]* [-stats]\n"
            "       src_dataset dst_dataset\n" );

    if( !bShort )
    {
        printf( "\n%s\n\n", GDALVersionInfo( "--version" ) );
        printf( "The following format drivers are configured and support output:\n" );
        for( iDr = 0; iDr < GDALGetDriverCount(); iDr++ )
        {
            GDALDriverH hDriver = GDALGetDriver(iDr);

            if( GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL ) != NULL
                    || GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATECOPY,
                                            NULL ) != NULL )
            {
                printf( "  %s: %s\n",
                        GDALGetDriverShortName( hDriver ),
                        GDALGetDriverLongName( hDriver ) );
            }
        }
    }

    if( pszErrorMsg != NULL )
        fprintf(stderr, "\nFAILURE: %s\n", pszErrorMsg);

    exit(1);
}

/* -------------------------------------------------------------------- */
/*                      CheckExtensionConsistency()                     */
/*                                                                      */
/*      Check that the target file extension is consistant with the     */
/*      requested driver. Actually, we only warn in cases where the     */
/*      inconsistency is blatant (use of an extension declared by one   */
/*      or several drivers, and not by the selected one)                */
/* -------------------------------------------------------------------- */


void CheckExtensionConsistency(const char* pszDestFilename,
                               const char* pszDriverName)
{

    char* pszDestExtension = CPLStrdup(CPLGetExtension(pszDestFilename));
    if (pszDestExtension[0] != '\0')
    {
        int nDriverCount = GDALGetDriverCount();
        CPLString osConflictingDriverList;
        for(int i=0;i<nDriverCount;i++)
        {
            GDALDriverH hDriver = GDALGetDriver(i);
            const char* pszDriverExtension =
                    GDALGetMetadataItem( hDriver, GDAL_DMD_EXTENSION, NULL );
            if (pszDriverExtension && EQUAL(pszDestExtension, pszDriverExtension))
            {
                if (GDALGetDriverByName(pszDriverName) != hDriver)
                {
                    if (osConflictingDriverList.size())
                        osConflictingDriverList += ", ";
                    osConflictingDriverList += GDALGetDriverShortName(hDriver);
                }
                else
                {
                    /* If the request driver allows the used extension, then */
                    /* just stop iterating now */
                    osConflictingDriverList = "";
                    break;
                }
            }
        }
        if (osConflictingDriverList.size())
        {
            fprintf(stderr,
                    "Warning: The target file has a '%s' extension, which is normally used by the %s driver%s,\n"
                    "but the requested output driver is %s. Is it really what you want ?\n",
                    pszDestExtension,
                    osConflictingDriverList.c_str(),
                    strchr(osConflictingDriverList.c_str(), ',') ? "s" : "",
                    pszDriverName);
        }
    }

    CPLFree(pszDestExtension);
}

/* -------------------------------------------------------------------- */
/*                        EarlySetConfigOptions()                       */
/* -------------------------------------------------------------------- */

void EarlySetConfigOptions( int argc, char ** argv )
{
    /* Must process some config options before GDALAllRegister() or OGRRegisterAll(), */
    /* but we can't call GDALGeneralCmdLineProcessor() or OGRGeneralCmdLineProcessor(), */
    /* because it needs the drivers to be registered for the --format or --formats options */
    for( int i = 1; i < argc; i++ )
    {
        if( EQUAL(argv[i],"--config") && i + 2 < argc &&
                (EQUAL(argv[i + 1], "GDAL_SKIP") ||
                 EQUAL(argv[i + 1], "GDAL_DRIVER_PATH") ||
                 EQUAL(argv[i + 1], "OGR_SKIP") ||
                 EQUAL(argv[i + 1], "OGR_DRIVER_PATH")) )
        {
            CPLSetConfigOption( argv[i+1], argv[i+2] );

            i += 2;
        }
    }
}


/************************************************************************/
/*                              SrcToDst()                              */
/************************************************************************/

static void SrcToDst( double dfX, double dfY,
                      int nSrcXOff, int nSrcYOff,
                      int nSrcXSize, int nSrcYSize,
                      int nDstXOff, int nDstYOff,
                      int nDstXSize, int nDstYSize,
                      double &dfXOut, double &dfYOut )

{
    dfXOut = ((dfX - nSrcXOff) / nSrcXSize) * nDstXSize + nDstXOff;
    dfYOut = ((dfY - nSrcYOff) / nSrcYSize) * nDstYSize + nDstYOff;
}

/************************************************************************/
/*                          GetSrcDstWindow()                           */
/************************************************************************/

static int FixSrcDstWindow( int* panSrcWin, int* panDstWin,
                            int nSrcRasterXSize,
                            int nSrcRasterYSize )

{
    const int nSrcXOff = panSrcWin[0];
    const int nSrcYOff = panSrcWin[1];
    const int nSrcXSize = panSrcWin[2];
    const int nSrcYSize = panSrcWin[3];

    const int nDstXOff = panDstWin[0];
    const int nDstYOff = panDstWin[1];
    const int nDstXSize = panDstWin[2];
    const int nDstYSize = panDstWin[3];

    int bModifiedX = FALSE, bModifiedY = FALSE;

    int nModifiedSrcXOff = nSrcXOff;
    int nModifiedSrcYOff = nSrcYOff;

    int nModifiedSrcXSize = nSrcXSize;
    int nModifiedSrcYSize = nSrcYSize;

    /* -------------------------------------------------------------------- */
    /*      Clamp within the bounds of the available source data.           */
    /* -------------------------------------------------------------------- */
    if( nModifiedSrcXOff < 0 )
    {
        nModifiedSrcXSize += nModifiedSrcXOff;
        nModifiedSrcXOff = 0;

        bModifiedX = TRUE;
    }

    if( nModifiedSrcYOff < 0 )
    {
        nModifiedSrcYSize += nModifiedSrcYOff;
        nModifiedSrcYOff = 0;
        bModifiedY = TRUE;
    }

    if( nModifiedSrcXOff + nModifiedSrcXSize > nSrcRasterXSize )
    {
        nModifiedSrcXSize = nSrcRasterXSize - nModifiedSrcXOff;
        bModifiedX = TRUE;
    }

    if( nModifiedSrcYOff + nModifiedSrcYSize > nSrcRasterYSize )
    {
        nModifiedSrcYSize = nSrcRasterYSize - nModifiedSrcYOff;
        bModifiedY = TRUE;
    }

    /* -------------------------------------------------------------------- */
    /*      Don't do anything if the requesting region is completely off    */
    /*      the source image.                                               */
    /* -------------------------------------------------------------------- */
    if( nModifiedSrcXOff >= nSrcRasterXSize
            || nModifiedSrcYOff >= nSrcRasterYSize
            || nModifiedSrcXSize <= 0 || nModifiedSrcYSize <= 0 )
    {
        return FALSE;
    }

    panSrcWin[0] = nModifiedSrcXOff;
    panSrcWin[1] = nModifiedSrcYOff;
    panSrcWin[2] = nModifiedSrcXSize;
    panSrcWin[3] = nModifiedSrcYSize;

    /* -------------------------------------------------------------------- */
    /*      If we haven't had to modify the source rectangle, then the      */
    /*      destination rectangle must be the whole region.                 */
    /* -------------------------------------------------------------------- */
    if( !bModifiedX && !bModifiedY )
        return TRUE;

    /* -------------------------------------------------------------------- */
    /*      Now transform this possibly reduced request back into the       */
    /*      destination buffer coordinates in case the output region is     */
    /*      less than the whole buffer.                                     */
    /* -------------------------------------------------------------------- */
    double dfDstULX, dfDstULY, dfDstLRX, dfDstLRY;

    SrcToDst( nModifiedSrcXOff, nModifiedSrcYOff,
              nSrcXOff, nSrcYOff,
              nSrcXSize, nSrcYSize,
              nDstXOff, nDstYOff,
              nDstXSize, nDstYSize,
              dfDstULX, dfDstULY );
    SrcToDst( nModifiedSrcXOff + nModifiedSrcXSize, nModifiedSrcYOff + nModifiedSrcYSize,
              nSrcXOff, nSrcYOff,
              nSrcXSize, nSrcYSize,
              nDstXOff, nDstYOff,
              nDstXSize, nDstYSize,
              dfDstLRX, dfDstLRY );

    int nModifiedDstXOff = nDstXOff;
    int nModifiedDstYOff = nDstYOff;
    int nModifiedDstXSize = nDstXSize;
    int nModifiedDstYSize = nDstYSize;

    if( bModifiedX )
    {
        nModifiedDstXOff = (int) ((dfDstULX - nDstXOff)+0.001);
        nModifiedDstXSize = (int) ((dfDstLRX - nDstXOff)+0.001)
                - nModifiedDstXOff;

        nModifiedDstXOff = MAX(0,nModifiedDstXOff);
        if( nModifiedDstXOff + nModifiedDstXSize > nDstXSize )
            nModifiedDstXSize = nDstXSize - nModifiedDstXOff;
    }

    if( bModifiedY )
    {
        nModifiedDstYOff = (int) ((dfDstULY - nDstYOff)+0.001);
        nModifiedDstYSize = (int) ((dfDstLRY - nDstYOff)+0.001)
                - nModifiedDstYOff;

        nModifiedDstYOff = MAX(0,nModifiedDstYOff);
        if( nModifiedDstYOff + nModifiedDstYSize > nDstYSize )
            nModifiedDstYSize = nDstYSize - nModifiedDstYOff;
    }

    if( nModifiedDstXSize < 1 || nModifiedDstYSize < 1 )
        return FALSE;
    else
    {
        panDstWin[0] = nModifiedDstXOff;
        panDstWin[1] = nModifiedDstYOff;
        panDstWin[2] = nModifiedDstXSize;
        panDstWin[3] = nModifiedDstYSize;

        return TRUE;
    }
}

enum
{
    MASK_DISABLED,
    MASK_AUTO,
    MASK_USER
};

#define CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(nExtraArg) \
    do { if (i + nExtraArg >= argc) \
    Usage(CPLSPrintf("%s option requires %d argument(s)", argv[i], nExtraArg)); } while(0)




/************************************************************************/
/*                            ArgIsNumeric()                            */
/************************************************************************/

int ArgIsNumeric( const char *pszArg )

{
    return CPLGetValueType(pszArg) != CPL_VALUE_STRING;
}

/************************************************************************/
/*                           AttachMetadata()                           */
/************************************************************************/

static void AttachMetadata( GDALDatasetH hDS, char **papszMetadataOptions )

{
    int nCount = CSLCount(papszMetadataOptions);
    int i;

    for( i = 0; i < nCount; i++ )
    {
        char    *pszKey = NULL;
        const char *pszValue;

        pszValue = CPLParseNameValue( papszMetadataOptions[i], &pszKey );
        GDALSetMetadataItem(hDS,pszKey,pszValue,NULL);
        CPLFree( pszKey );
    }

    CSLDestroy( papszMetadataOptions );
}

/************************************************************************/
/*                           CopyBandInfo()                            */
/************************************************************************/

/* A bit of a clone of VRTRasterBand::CopyCommonInfoFrom(), but we need */
/* more and more custom behaviour in the context of gdal_translate ... */

static void CopyBandInfo( GDALRasterBand * poSrcBand, GDALRasterBand * poDstBand,
                          int bCanCopyStatsMetadata, int bCopyScale, int bCopyNoData )

{
    int bSuccess;
    double dfNoData;

    if (bCanCopyStatsMetadata)
    {
        poDstBand->SetMetadata( poSrcBand->GetMetadata() );
    }
    else
    {
        char** papszMetadata = poSrcBand->GetMetadata();
        char** papszMetadataNew = NULL;
        for( int i = 0; papszMetadata != NULL && papszMetadata[i] != NULL; i++ )
        {
            if (strncmp(papszMetadata[i], "STATISTICS_", 11) != 0)
                papszMetadataNew = CSLAddString(papszMetadataNew, papszMetadata[i]);
        }
        poDstBand->SetMetadata( papszMetadataNew );
        CSLDestroy(papszMetadataNew);
    }

    poDstBand->SetColorTable( poSrcBand->GetColorTable() );
    poDstBand->SetColorInterpretation(poSrcBand->GetColorInterpretation());
    if( strlen(poSrcBand->GetDescription()) > 0 )
        poDstBand->SetDescription( poSrcBand->GetDescription() );

    if (bCopyNoData)
    {
        dfNoData = poSrcBand->GetNoDataValue( &bSuccess );
        if( bSuccess )
            poDstBand->SetNoDataValue( dfNoData );
    }

    if (bCopyScale)
    {
        poDstBand->SetOffset( poSrcBand->GetOffset() );
        poDstBand->SetScale( poSrcBand->GetScale() );
    }

    poDstBand->SetCategoryNames( poSrcBand->GetCategoryNames() );
    if( !EQUAL(poSrcBand->GetUnitType(),"") )
        poDstBand->SetUnitType( poSrcBand->GetUnitType() );

}


// Beginning of class implementation ************************************************************************

RasterGeoreferencer::RasterGeoreferencer():pszFormat("GTiff")
{
    bFormatExplicitelySet = FALSE;
    panBandList = NULL; /* negative value of panBandList[i] means mask band of ABS(panBandList[i]) */
    nBandCount = 0;
    bDefBands = TRUE;
    adfGeoTransform = (double*)malloc(6*sizeof(double));
    eOutputType = GDT_Unknown;
    nOXSize = 0;
    nOYSize = 0;
    pszOXSize=NULL;
    pszOYSize=NULL;
    papszCreateOptions = NULL;
    anSrcWin = (int*)malloc(4*sizeof(int));
    bStrict = FALSE;
    bScale = FALSE;
    bHaveScaleSrc = FALSE;
    bUnscale=FALSE;
    dfScaleSrcMin=0.0;
    dfScaleSrcMax=255.0;
    dfScaleDstMin=0.0;
    dfScaleDstMax=255.0;
    papszMetadataOptions = NULL;
    pszOutputSRS = NULL;
    bQuiet = FALSE;
    bGotBounds = FALSE;
    pfnProgress = GDALTermProgress;
    nGCPCount = 0;
    pasGCPs = NULL;
    iSrcFileArg = -1;
    iDstFileArg = -1;
    bCopySubDatasets = FALSE;
    adfULLR = (double*)malloc(4*sizeof(double));
    adfULLR[0]=0; adfULLR[1]=0; adfULLR[2]=0; adfULLR[3]=0;
    bSetNoData = FALSE;
    bUnsetNoData = FALSE;
    dfNoDataReal = 0.0;
    nRGBExpand = 0;
    bParsedMaskArgument = FALSE;
    eMaskMode = MASK_AUTO;
    nMaskBand = 0; /* negative value means mask band of ABS(nMaskBand) */
    bStats = FALSE, bApproxStats = FALSE;
    bErrorOnPartiallyOutside = FALSE;
    bErrorOnCompletelyOutside = FALSE;
    pszDest = NULL;

}

RasterGeoreferencer::~RasterGeoreferencer()
{
    free(adfGeoTransform);
    free(anSrcWin);
    free(adfULLR);
}

int RasterGeoreferencer::WriteGeoFile(Mat &raster, Mat &rasterMask, QString outputFile, QString cmdLineOptions)
{



    // Complete arguments for exe name and mask
    QString localArgv;
    if(raster.channels()>1){
        localArgv= QString("WriteGeoFile ") + cmdLineOptions + QString(" -mask 4 --config GDAL_TIFF_INTERNAL_MASK YES ") + outputFile;
    }else{
        localArgv= QString("WriteGeoFile ") + cmdLineOptions + QString(" -mask 2 --config GDAL_TIFF_INTERNAL_MASK YES ") + outputFile;
    }

    fprintf(stderr,"%s\n", localArgv.toStdString().c_str());

    // recreate argc and argv for options parsing **************************************

    QStringList cmdQuoteSplitList = localArgv.split("\"");
    QStringList argvList;

    int i=1;

    // isolate quote separated argument as a unique argument and split others on space
    foreach(QString arguments, cmdQuoteSplitList){

        if (i%2 == 0){
            argvList += arguments;
        }else{
            argvList += arguments.split(" ",QString::SkipEmptyParts);
        }
        i++;
    }
    argvList.removeAll(" ");

    int argc = argvList.length();
    char **argv;
    argv = (char **)malloc(argc*sizeof(char*));

    i=0;
    foreach(QString argument, argvList){

        argv[i] = (char*) malloc(sizeof(char)*argument.size());
        strcpy ( argv[i], argument.toStdString().c_str() );
        i++;
    }

    // *********************************************************************************

    anSrcWin[0] = 0;
    anSrcWin[1] = 0;
    anSrcWin[2] = 0;
    anSrcWin[3] = 0;

    dfULX = dfULY = dfLRX = dfLRY = 0.0;

    /* Check strict compilation and runtime library version as we use C++ API */
    if (! GDAL_CHECK_VERSION(argv[0]))
        exit(1);

    EarlySetConfigOptions(argc, argv);

    /* -------------------------------------------------------------------- */
    /*      Register standard GDAL drivers, and process generic GDAL        */
    /*      command options.                                                */
    /* -------------------------------------------------------------------- */
    GDALAllRegister();
    argc = GDALGeneralCmdLineProcessor( argc, &argv, 0 );
    if( argc < 1 )
        exit( -argc );

    /* -------------------------------------------------------------------- */
    /*      Handle command line arguments.                                  */
    /* -------------------------------------------------------------------- */
    for( i = 1; i < argc; i++ )
    {
        if( EQUAL(argv[i], "--utility_version") )
        {
            printf("%s was compiled against GDAL %s and is running against GDAL %s\n",
                   argv[0], GDAL_RELEASE_NAME, GDALVersionInfo("RELEASE_NAME"));
            return 0;
        }
        else if( EQUAL(argv[i],"--help") )
            Usage();
        else if ( EQUAL(argv[i], "--long-usage") )
        {
            Usage(NULL, FALSE);
        }
        else if( EQUAL(argv[i],"-of") && i < argc-1 )
        {
            pszFormat = argv[++i];
            bFormatExplicitelySet = TRUE;
        }

        else if( EQUAL(argv[i],"-q") || EQUAL(argv[i],"-quiet") )
        {
            bQuiet = TRUE;
            pfnProgress = GDALDummyProgress;
        }

        else if( EQUAL(argv[i],"-ot") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            int	iType;

            for( iType = 1; iType < GDT_TypeCount; iType++ )
            {
                if( GDALGetDataTypeName((GDALDataType)iType) != NULL
                        && EQUAL(GDALGetDataTypeName((GDALDataType)iType),
                                 argv[i+1]) )
                {
                    eOutputType = (GDALDataType) iType;
                }
            }

            if( eOutputType == GDT_Unknown )
            {
                Usage(CPLSPrintf("Unknown output pixel type: %s.", argv[i+1] ));
            }
            i++;
        }
        else if( EQUAL(argv[i],"-b") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            const char* pszBand = argv[i+1];
            int bMask = FALSE;
            if (EQUAL(pszBand, "mask"))
                pszBand = "mask,1";
            if (EQUALN(pszBand, "mask,", 5))
            {
                bMask = TRUE;
                pszBand += 5;
                /* If we use tha source mask band as a regular band */
                /* don't create a target mask band by default */
                if( !bParsedMaskArgument )
                    eMaskMode = MASK_DISABLED;
            }
            int nBand = atoi(pszBand);
            if( nBand < 1 )
            {
                Usage(CPLSPrintf( "Unrecognizable band number (%s).", argv[i+1] ));
            }
            i++;

            nBandCount++;
            panBandList = (int *)
                    CPLRealloc(panBandList, sizeof(int) * nBandCount);
            panBandList[nBandCount-1] = nBand;
            if (bMask)
                panBandList[nBandCount-1] *= -1;

            if( panBandList[nBandCount-1] != nBandCount )
                bDefBands = FALSE;
        }
        else if( EQUAL(argv[i],"-mask") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            bParsedMaskArgument = TRUE;
            const char* pszBand = argv[i+1];
            if (EQUAL(pszBand, "none"))
            {
                eMaskMode = MASK_DISABLED;
            }
            else if (EQUAL(pszBand, "auto"))
            {
                eMaskMode = MASK_AUTO;
            }
            else
            {
                int bMask = FALSE;
                if (EQUAL(pszBand, "mask"))
                    pszBand = "mask,1";
                if (EQUALN(pszBand, "mask,", 5))
                {
                    bMask = TRUE;
                    pszBand += 5;
                }
                int nBand = atoi(pszBand);
                if( nBand < 1 )
                {
                    Usage(CPLSPrintf( "Unrecognizable band number (%s).", argv[i+1] ));
                }

                eMaskMode = MASK_USER;
                nMaskBand = nBand;
                if (bMask)
                    nMaskBand *= -1;
            }
            i ++;
        }
        else if( EQUAL(argv[i],"-not_strict")  )
            bStrict = FALSE;

        else if( EQUAL(argv[i],"-strict")  )
            bStrict = TRUE;

        else if( EQUAL(argv[i],"-sds")  )
            bCopySubDatasets = TRUE;

        else if( EQUAL(argv[i],"-gcp") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(4);
            char* endptr = NULL;
            /* -gcp pixel line easting northing [elev] */

            nGCPCount++;
            pasGCPs = (GDAL_GCP *)
                    CPLRealloc( pasGCPs, sizeof(GDAL_GCP) * nGCPCount );
            GDALInitGCPs( 1, pasGCPs + nGCPCount - 1 );

            pasGCPs[nGCPCount-1].dfGCPPixel = CPLAtofM(argv[++i]);
            pasGCPs[nGCPCount-1].dfGCPLine = CPLAtofM(argv[++i]);
            pasGCPs[nGCPCount-1].dfGCPX = CPLAtofM(argv[++i]);
            pasGCPs[nGCPCount-1].dfGCPY = CPLAtofM(argv[++i]);
            if( argv[i+1] != NULL
                    && (CPLStrtod(argv[i+1], &endptr) != 0.0 || argv[i+1][0] == '0') )
            {
                /* Check that last argument is really a number and not a filename */
                /* looking like a number (see ticket #863) */
                if (endptr && *endptr == 0)
                    pasGCPs[nGCPCount-1].dfGCPZ = CPLAtofM(argv[++i]);
            }

            /* should set id and info? */
        }

        else if( EQUAL(argv[i],"-a_nodata") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            if (EQUAL(argv[i+1], "none"))
            {
                bUnsetNoData = TRUE;
            }
            else
            {
                bSetNoData = TRUE;
                dfNoDataReal = CPLAtofM(argv[i+1]);
            }
            i += 1;
        }

        else if( EQUAL(argv[i],"-a_ullr") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(4);
            adfULLR[0] = CPLAtofM(argv[i+1]);
            adfULLR[1] = CPLAtofM(argv[i+2]);
            adfULLR[2] = CPLAtofM(argv[i+3]);
            adfULLR[3] = CPLAtofM(argv[i+4]);

            bGotBounds = TRUE;

            i += 4;
        }

        else if( EQUAL(argv[i],"-co") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            papszCreateOptions = CSLAddString( papszCreateOptions, argv[++i] );
        }

        else if( EQUAL(argv[i],"-scale") )
        {
            bScale = TRUE;
            if( i < argc-2 && ArgIsNumeric(argv[i+1]) )
            {
                bHaveScaleSrc = TRUE;
                dfScaleSrcMin = CPLAtofM(argv[i+1]);
                dfScaleSrcMax = CPLAtofM(argv[i+2]);
                i += 2;
            }
            if( i < argc-2 && bHaveScaleSrc && ArgIsNumeric(argv[i+1]) )
            {
                dfScaleDstMin = CPLAtofM(argv[i+1]);
                dfScaleDstMax = CPLAtofM(argv[i+2]);
                i += 2;
            }
            else
            {
                dfScaleDstMin = 0.0;
                dfScaleDstMax = 255.999;
            }
        }

        else if( EQUAL(argv[i], "-unscale") )
        {
            bUnscale = TRUE;
        }

        else if( EQUAL(argv[i],"-mo") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            papszMetadataOptions = CSLAddString( papszMetadataOptions,
                                                 argv[++i] );
        }

        else if( EQUAL(argv[i],"-outsize") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(2);
            pszOXSize = argv[++i];
            pszOYSize = argv[++i];
        }

        else if( EQUAL(argv[i],"-srcwin") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(4);
            anSrcWin[0] = atoi(argv[++i]);
            anSrcWin[1] = atoi(argv[++i]);
            anSrcWin[2] = atoi(argv[++i]);
            anSrcWin[3] = atoi(argv[++i]);
        }

        else if( EQUAL(argv[i],"-projwin") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(4);
            dfULX = CPLAtofM(argv[++i]);
            dfULY = CPLAtofM(argv[++i]);
            dfLRX = CPLAtofM(argv[++i]);
            dfLRY = CPLAtofM(argv[++i]);
        }

        else if( EQUAL(argv[i],"-epo") )
        {
            bErrorOnPartiallyOutside = TRUE;
            bErrorOnCompletelyOutside = TRUE;
        }

        else  if( EQUAL(argv[i],"-eco") )
        {
            bErrorOnCompletelyOutside = TRUE;
        }

        else if( EQUAL(argv[i],"-a_srs") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            OGRSpatialReference oOutputSRS;

            if( oOutputSRS.SetFromUserInput( argv[i+1] ) != OGRERR_NONE )
            {
                fprintf( stderr, "Failed to process SRS definition: %s\n",
                         argv[i+1] );
                GDALDestroyDriverManager();
                exit( 1 );
            }

            oOutputSRS.exportToWkt( &pszOutputSRS );
            i++;
        }

        else if( EQUAL(argv[i],"-expand") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            if (EQUAL(argv[i+1], "gray"))
                nRGBExpand = 1;
            else if (EQUAL(argv[i+1], "rgb"))
                nRGBExpand = 3;
            else if (EQUAL(argv[i+1], "rgba"))
                nRGBExpand = 4;
            else
            {
                Usage(CPLSPrintf( "Value %s unsupported. Only gray, rgb or rgba are supported.",
                                  argv[i] ));
            }
            i++;
        }

        else if( EQUAL(argv[i], "-stats") )
        {
            bStats = TRUE;
            bApproxStats = FALSE;
        }
        else if( EQUAL(argv[i], "-approx_stats") )
        {
            bStats = TRUE;
            bApproxStats = TRUE;
        }

        else if( argv[i][0] == '-' )
        {
            Usage(CPLSPrintf("Unkown option name '%s'", argv[i]));
        }
        else if( pszDest == NULL )
        {
            pszDest = argv[i];
            iDstFileArg = i;
            printf( "pszDest = %s \n", pszDest );
        }

        else
        {
            Usage("Too many command options.");
        }
    }

    if( pszDest == NULL )
    {
        Usage("No target dataset specified.");
    }

    if( strcmp(pszDest, "/vsistdout/") == 0)
    {
        bQuiet = TRUE;
        pfnProgress = GDALDummyProgress;
    }

    if (!bQuiet && !bFormatExplicitelySet)
        CheckExtensionConsistency(pszDest, pszFormat);

    /* -------------------------------------------------------------------- */
    /*      Attempt to open source file.                                    */
    /* -------------------------------------------------------------------- */

    cout <<  "Nb of registered drivers = " << GetGDALDriverManager()->GetDriverCount() << std::endl ;
    inputDriver = GetGDALDriverManager()->GetDriverByName("MEM");

    if(! inputDriver)                              // Check for invalid input
    {
        cout <<  "Could not create the input driver" << std::endl ;
        return -1;
    }


    if(! raster.data || ! rasterMask.data)                              // Check for invalid input
    {
        cout <<  "Raster data is incomplete" << std::endl ;
        return -1;
    }


    GDALRasterBand *pBand;
    if (raster.channels()>1){
        poDataset = inputDriver->Create ("", raster.cols, raster.rows, 4, GDT_Byte, NULL);
    }else{
        poDataset = inputDriver->Create ("", raster.cols, raster.rows, 2, GDT_Byte, NULL);
    }

    //Copy raster data & mask
    if (raster.channels()>1){

        vector<Mat> rasterChannels(3);
        split(raster, rasterChannels);

        for(int k=0;k<3;k++){
            pBand = poDataset->GetRasterBand(3-k);
            pBand->RasterIO(GF_Write,0,0,raster.cols,raster.rows,rasterChannels[k].data,raster.cols,raster.rows,GDT_Byte,0,0);
        }

        pBand = poDataset->GetRasterBand(4);
        pBand->RasterIO(GF_Write,0,0,rasterMask.cols,rasterMask.rows,rasterMask.data,raster.cols,raster.rows,GDT_Byte,0,0);

    }else{
        pBand = poDataset->GetRasterBand(1);
        pBand->RasterIO(GF_Write,0,0,raster.cols,raster.rows,raster.data,raster.cols,raster.rows,GDT_Byte,0,0);

        pBand = poDataset->GetRasterBand(2);
        pBand->RasterIO(GF_Write,0,0,raster.cols,raster.rows,rasterMask.data,raster.cols,raster.rows,GDT_Byte,0,0);
    }



    hDataset = poDataset; // C++ -> C casting for retrocompat

    if( hDataset == NULL )
    {
        fprintf( stderr,
                 "GDALOpen failed - %d\n%s\n",
                 CPLGetLastErrorNo(), CPLGetLastErrorMsg() );
        GDALDestroyDriverManager();
        exit( 1 );
    }



    /* -------------------------------------------------------------------- */
    /*      Collect some information from the source file.                  */
    /* -------------------------------------------------------------------- */
    nRasterXSize = GDALGetRasterXSize( hDataset );
    nRasterYSize = GDALGetRasterYSize( hDataset );

    if( !bQuiet )
        printf( "Input file size is %d, %d\n", nRasterXSize, nRasterYSize );

    if( anSrcWin[2] == 0 && anSrcWin[3] == 0 )
    {
        anSrcWin[2] = nRasterXSize;
        anSrcWin[3] = nRasterYSize;
    }

    /* -------------------------------------------------------------------- */
    /*	Build band list to translate					*/
    /* -------------------------------------------------------------------- */
    if( nBandCount == 0 )
    {
        nBandCount = GDALGetRasterCount( hDataset );
        if( nBandCount == 0 )
        {
            fprintf( stderr, "Input file has no bands, and so cannot be translated.\n" );
            GDALDestroyDriverManager();
            exit(1 );
        }

        panBandList = (int *) CPLMalloc(sizeof(int)*nBandCount);
        for( i = 0; i < nBandCount; i++ )
            panBandList[i] = i+1;
    }
    else
    {
        for( i = 0; i < nBandCount; i++ )
        {
            if( ABS(panBandList[i]) > GDALGetRasterCount(hDataset) )
            {
                fprintf( stderr,
                         "Band %d requested, but only bands 1 to %d available.\n",
                         ABS(panBandList[i]), GDALGetRasterCount(hDataset) );
                GDALDestroyDriverManager();
                exit( 2 );
            }
        }

        if( nBandCount != GDALGetRasterCount( hDataset ) )
            bDefBands = FALSE;
    }

    /* -------------------------------------------------------------------- */
    /*      Compute the source window from the projected source window      */
    /*      if the projected coordinates were provided.  Note that the      */
    /*      projected coordinates are in ulx, uly, lrx, lry format,         */
    /*      while the anSrcWin is xoff, yoff, xsize, ysize with the         */
    /*      xoff,yoff being the ulx, uly in pixel/line.                     */
    /* -------------------------------------------------------------------- */
    if( dfULX != 0.0 || dfULY != 0.0
            || dfLRX != 0.0 || dfLRY != 0.0 )
    {
        double	adfGeoTransform[6];

        GDALGetGeoTransform( hDataset, adfGeoTransform );

        if( adfGeoTransform[2] != 0.0 || adfGeoTransform[4] != 0.0 )
        {
            fprintf( stderr,
                     "The -projwin option was used, but the geotransform is\n"
                     "rotated.  This configuration is not supported.\n" );
            GDALClose( hDataset );
            CPLFree( panBandList );
            GDALDestroyDriverManager();
            exit( 1 );
        }

        anSrcWin[0] = (int)
                ((dfULX - adfGeoTransform[0]) / adfGeoTransform[1] + 0.001);
        anSrcWin[1] = (int)
                ((dfULY - adfGeoTransform[3]) / adfGeoTransform[5] + 0.001);

        anSrcWin[2] = (int) ((dfLRX - dfULX) / adfGeoTransform[1] + 0.5);
        anSrcWin[3] = (int) ((dfLRY - dfULY) / adfGeoTransform[5] + 0.5);

        if( !bQuiet )
            fprintf( stdout,
                     "Computed -srcwin %d %d %d %d from projected window.\n",
                     anSrcWin[0],
                    anSrcWin[1],
                    anSrcWin[2],
                    anSrcWin[3] );
    }

    /* -------------------------------------------------------------------- */
    /*      Verify source window dimensions.                                */
    /* -------------------------------------------------------------------- */
    if( anSrcWin[2] <= 0 || anSrcWin[3] <= 0 )
    {
        fprintf( stderr,
                 "Error: %s-srcwin %d %d %d %d has negative width and/or height.\n",
                 ( dfULX != 0.0 || dfULY != 0.0 || dfLRX != 0.0 || dfLRY != 0.0 ) ? "Computed " : "",
                 anSrcWin[0],
                anSrcWin[1],
                anSrcWin[2],
                anSrcWin[3] );
        exit( 1 );
    }

    /* -------------------------------------------------------------------- */
    /*      Verify source window dimensions.                                */
    /* -------------------------------------------------------------------- */
    else if( anSrcWin[0] < 0 || anSrcWin[1] < 0
             || anSrcWin[0] + anSrcWin[2] > GDALGetRasterXSize(hDataset)
             || anSrcWin[1] + anSrcWin[3] > GDALGetRasterYSize(hDataset) )
    {
        int bCompletelyOutside = anSrcWin[0] + anSrcWin[2] <= 0 ||
                anSrcWin[1] + anSrcWin[3] <= 0 ||
                anSrcWin[0] >= GDALGetRasterXSize(hDataset) ||
                anSrcWin[1] >= GDALGetRasterYSize(hDataset);
        int bIsError = bErrorOnPartiallyOutside || (bCompletelyOutside && bErrorOnCompletelyOutside);
        if( !bQuiet || bIsError )
        {
            fprintf( stderr,
                     "%s: %s-srcwin %d %d %d %d falls %s outside raster extent.%s\n",
                     (bIsError) ? "Error" : "Warning",
                     ( dfULX != 0.0 || dfULY != 0.0 || dfLRX != 0.0 || dfLRY != 0.0 ) ? "Computed " : "",
                     anSrcWin[0],
                    anSrcWin[1],
                    anSrcWin[2],
                    anSrcWin[3],
                    (bCompletelyOutside) ? "completely" : "partially",
                    (bIsError) ? "" : " Going on however." );
        }
        if( bIsError )
            exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*      Find the output driver.                                         */
    /* -------------------------------------------------------------------- */
    hDriver = GDALGetDriverByName( pszFormat );
    if( hDriver == NULL )
    {
        int	iDr;

        printf( "Output driver `%s' not recognised.\n", pszFormat );
        printf( "The following format drivers are configured and support output:\n" );
        for( iDr = 0; iDr < GDALGetDriverCount(); iDr++ )
        {
            GDALDriverH hDriver = GDALGetDriver(iDr);

            if( GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL ) != NULL
                    || GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATECOPY,
                                            NULL ) != NULL )
            {
                printf( "  %s: %s\n",
                        GDALGetDriverShortName( hDriver  ),
                        GDALGetDriverLongName( hDriver ) );
            }
        }
        printf( "\n" );
        Usage();

        GDALClose( hDataset );
        CPLFree( panBandList );
        GDALDestroyDriverManager();
        CSLDestroy( argv );
        CSLDestroy( papszCreateOptions );
        exit( 1 );
    }

    /* -------------------------------------------------------------------- */
    /*      The short form is to CreateCopy().  We use this if the input    */
    /*      matches the whole dataset.  Eventually we should rewrite        */
    /*      this entire program to use virtual datasets to construct a      */
    /*      virtual input source to copy from.                              */
    /* -------------------------------------------------------------------- */


    int bSpatialArrangementPreserved = (
                anSrcWin[0] == 0 && anSrcWin[1] == 0
            && anSrcWin[2] == GDALGetRasterXSize(hDataset)
            && anSrcWin[3] == GDALGetRasterYSize(hDataset)
            && pszOXSize == NULL && pszOYSize == NULL );

    if( eOutputType == GDT_Unknown
            && !bScale && !bUnscale
            && CSLCount(papszMetadataOptions) == 0 && bDefBands
            && eMaskMode == MASK_AUTO
            && bSpatialArrangementPreserved
            && nGCPCount == 0 && !bGotBounds
            && pszOutputSRS == NULL && !bSetNoData && !bUnsetNoData
            && nRGBExpand == 0 && !bStats )
    {

        hOutDS = GDALCreateCopy( hDriver, pszDest, hDataset,
                                 bStrict, papszCreateOptions,
                                 pfnProgress, NULL );

        if( hOutDS != NULL )
            GDALClose( hOutDS );

        GDALClose( hDataset );

        CPLFree( panBandList );

        if( !bSubCall )
        {
            GDALDumpOpenDatasets( stderr );
            GDALDestroyDriverManager();
        }

        CSLDestroy( argv );
        CSLDestroy( papszCreateOptions );

        return hOutDS == NULL;
    }

    /* -------------------------------------------------------------------- */
    /*      Establish some parameters.                                      */
    /* -------------------------------------------------------------------- */
    if( pszOXSize == NULL )
    {
        nOXSize = anSrcWin[2];
        nOYSize = anSrcWin[3];
    }
    else
    {
        nOXSize = (int) ((pszOXSize[strlen(pszOXSize)-1]=='%'
                         ? CPLAtofM(pszOXSize)/100*anSrcWin[2] : atoi(pszOXSize)));
        nOYSize = (int) ((pszOYSize[strlen(pszOYSize)-1]=='%'
                         ? CPLAtofM(pszOYSize)/100*anSrcWin[3] : atoi(pszOYSize)));
    }

    /* ==================================================================== */
    /*      Create a virtual dataset.                                       */
    /* ==================================================================== */
    VRTDataset *poVDS;

    /* -------------------------------------------------------------------- */
    /*      Make a virtual clone.                                           */
    /* -------------------------------------------------------------------- */
    poVDS = (VRTDataset *) VRTCreate( nOXSize, nOYSize );

    if( nGCPCount == 0 )
    {
        if( pszOutputSRS != NULL )
        {
            poVDS->SetProjection( pszOutputSRS );
        }
        else
        {
            pszProjection = GDALGetProjectionRef( hDataset );
            if( pszProjection != NULL && strlen(pszProjection) > 0 )
                poVDS->SetProjection( pszProjection );
        }
    }

    if( bGotBounds )
    {
        adfGeoTransform[0] = adfULLR[0];
        adfGeoTransform[1] = (adfULLR[2] - adfULLR[0]) / nOXSize;
        adfGeoTransform[2] = 0.0;
        adfGeoTransform[3] = adfULLR[1];
        adfGeoTransform[4] = 0.0;
        adfGeoTransform[5] = (adfULLR[3] - adfULLR[1]) / nOYSize;

        poVDS->SetGeoTransform( adfGeoTransform );
    }

    else if( GDALGetGeoTransform( hDataset, adfGeoTransform ) == CE_None
             && nGCPCount == 0 )
    {
        adfGeoTransform[0] += anSrcWin[0] * adfGeoTransform[1]
                + anSrcWin[1] * adfGeoTransform[2];
        adfGeoTransform[3] += anSrcWin[0] * adfGeoTransform[4]
                + anSrcWin[1] * adfGeoTransform[5];

        adfGeoTransform[1] *= anSrcWin[2] / (double) nOXSize;
        adfGeoTransform[2] *= anSrcWin[3] / (double) nOYSize;
        adfGeoTransform[4] *= anSrcWin[2] / (double) nOXSize;
        adfGeoTransform[5] *= anSrcWin[3] / (double) nOYSize;

        poVDS->SetGeoTransform( adfGeoTransform );
    }

    if( nGCPCount != 0 )
    {
        const char *pszGCPProjection = pszOutputSRS;

        if( pszGCPProjection == NULL )
            pszGCPProjection = GDALGetGCPProjection( hDataset );
        if( pszGCPProjection == NULL )
            pszGCPProjection = "";

        poVDS->SetGCPs( nGCPCount, pasGCPs, pszGCPProjection );

        GDALDeinitGCPs( nGCPCount, pasGCPs );
        CPLFree( pasGCPs );
    }

    else if( GDALGetGCPCount( hDataset ) > 0 )
    {
        GDAL_GCP *pasGCPs;
        int       nGCPs = GDALGetGCPCount( hDataset );

        pasGCPs = GDALDuplicateGCPs( nGCPs, GDALGetGCPs( hDataset ) );

        for( i = 0; i < nGCPs; i++ )
        {
            pasGCPs[i].dfGCPPixel -= anSrcWin[0];
            pasGCPs[i].dfGCPLine  -= anSrcWin[1];
            pasGCPs[i].dfGCPPixel *= (nOXSize / (double) anSrcWin[2] );
            pasGCPs[i].dfGCPLine  *= (nOYSize / (double) anSrcWin[3] );
        }

        poVDS->SetGCPs( nGCPs, pasGCPs,
                        GDALGetGCPProjection( hDataset ) );

        GDALDeinitGCPs( nGCPs, pasGCPs );
        CPLFree( pasGCPs );
    }

    /* -------------------------------------------------------------------- */
    /*      To make the VRT to look less awkward (but this is optional      */
    /*      in fact), avoid negative values.                                */
    /* -------------------------------------------------------------------- */
    int anDstWin[4];
    anDstWin[0] = 0;
    anDstWin[1] = 0;
    anDstWin[2] = nOXSize;
    anDstWin[3] = nOYSize;

    FixSrcDstWindow( anSrcWin, anDstWin,
                     GDALGetRasterXSize(hDataset),
                     GDALGetRasterYSize(hDataset) );

    /* -------------------------------------------------------------------- */
    /*      Transfer generally applicable metadata.                         */
    /* -------------------------------------------------------------------- */
    char** papszMetadata = CSLDuplicate(((GDALDataset*)hDataset)->GetMetadata());
    if ( bScale || bUnscale || eOutputType != GDT_Unknown )
    {
        /* Remove TIFFTAG_MINSAMPLEVALUE and TIFFTAG_MAXSAMPLEVALUE */
        /* if the data range may change because of options */
        char** papszIter = papszMetadata;
        while(papszIter && *papszIter)
        {
            if (EQUALN(*papszIter, "TIFFTAG_MINSAMPLEVALUE=", 23) ||
                    EQUALN(*papszIter, "TIFFTAG_MAXSAMPLEVALUE=", 23))
            {
                CPLFree(*papszIter);
                memmove(papszIter, papszIter+1, sizeof(char*) * (CSLCount(papszIter+1)+1));
            }
            else
                papszIter++;
        }
    }
    poVDS->SetMetadata( papszMetadata );
    CSLDestroy( papszMetadata );
    AttachMetadata( (GDALDatasetH) poVDS, papszMetadataOptions );

    const char* pszInterleave = GDALGetMetadataItem(hDataset, "INTERLEAVE", "IMAGE_STRUCTURE");
    if (pszInterleave)
        poVDS->SetMetadataItem("INTERLEAVE", pszInterleave, "IMAGE_STRUCTURE");

    /* -------------------------------------------------------------------- */
    /*      Transfer metadata that remains valid if the spatial             */
    /*      arrangement of the data is unaltered.                           */
    /* -------------------------------------------------------------------- */
    if( bSpatialArrangementPreserved )
    {
        char **papszMD;

        papszMD = ((GDALDataset*)hDataset)->GetMetadata("RPC");
        if( papszMD != NULL )
            poVDS->SetMetadata( papszMD, "RPC" );

        papszMD = ((GDALDataset*)hDataset)->GetMetadata("GEOLOCATION");
        if( papszMD != NULL )
            poVDS->SetMetadata( papszMD, "GEOLOCATION" );
    }

    int nSrcBandCount = nBandCount;

    if (nRGBExpand != 0)
    {
        GDALRasterBand  *poSrcBand;
        poSrcBand = ((GDALDataset *)
                     hDataset)->GetRasterBand(ABS(panBandList[0]));
        if (panBandList[0] < 0)
            poSrcBand = poSrcBand->GetMaskBand();
        GDALColorTable* poColorTable = poSrcBand->GetColorTable();
        if (poColorTable == NULL)
        {
            fprintf(stderr, "Error : band %d has no color table\n", ABS(panBandList[0]));
            GDALClose( hDataset );
            CPLFree( panBandList );
            GDALDestroyDriverManager();
            CSLDestroy( argv );
            CSLDestroy( papszCreateOptions );
            exit( 1 );
        }

        /* Check that the color table only contains gray levels */
        /* when using -expand gray */
        if (nRGBExpand == 1)
        {
            int nColorCount = poColorTable->GetColorEntryCount();
            int nColor;
            for( nColor = 0; nColor < nColorCount; nColor++ )
            {
                const GDALColorEntry* poEntry = poColorTable->GetColorEntry(nColor);
                if (poEntry->c1 != poEntry->c2 || poEntry->c1 != poEntry->c2)
                {
                    fprintf(stderr, "Warning : color table contains non gray levels colors\n");
                    break;
                }
            }
        }

        if (nBandCount == 1)
            nBandCount = nRGBExpand;
        else if (nBandCount == 2 && (nRGBExpand == 3 || nRGBExpand == 4))
            nBandCount = nRGBExpand;
        else
        {
            fprintf(stderr, "Error : invalid use of -expand option.\n");
            exit( 1 );
        }
    }

    int bFilterOutStatsMetadata =
            (bScale || bUnscale || !bSpatialArrangementPreserved || nRGBExpand != 0);

    /* ==================================================================== */
    /*      Process all bands.                                              */
    /* ==================================================================== */
    for( i = 0; i < nBandCount; i++ )
    {
        VRTSourcedRasterBand   *poVRTBand;
        GDALRasterBand  *poSrcBand;
        GDALDataType    eBandType;
        int             nComponent = 0;

        int nSrcBand;
        if (nRGBExpand != 0)
        {
            if (nSrcBandCount == 2 && nRGBExpand == 4 && i == 3)
                nSrcBand = panBandList[1];
            else
            {
                nSrcBand = panBandList[0];
                nComponent = i + 1;
            }
        }
        else
            nSrcBand = panBandList[i];

        poSrcBand = ((GDALDataset *) hDataset)->GetRasterBand(ABS(nSrcBand));

        /* -------------------------------------------------------------------- */
        /*      Select output data type to match source.                        */
        /* -------------------------------------------------------------------- */
        if( eOutputType == GDT_Unknown )
            eBandType = poSrcBand->GetRasterDataType();
        else
            eBandType = eOutputType;

        /* -------------------------------------------------------------------- */
        /*      Create this band.                                               */
        /* -------------------------------------------------------------------- */
        poVDS->AddBand( eBandType, NULL );
        poVRTBand = (VRTSourcedRasterBand *) poVDS->GetRasterBand( i+1 );
        if (nSrcBand < 0)
        {
            poVRTBand->AddMaskBandSource(poSrcBand,
                                         anSrcWin[0], anSrcWin[1],
                    anSrcWin[2], anSrcWin[3],
                    anDstWin[0], anDstWin[1],
                    anDstWin[2], anDstWin[3]);
            continue;
        }

        /* -------------------------------------------------------------------- */
        /*      Do we need to collect scaling information?                      */
        /* -------------------------------------------------------------------- */
        double dfScale=1.0, dfOffset=0.0;

        if( bScale && !bHaveScaleSrc )
        {
            double	adfCMinMax[2];
            GDALComputeRasterMinMax( poSrcBand, TRUE, adfCMinMax );
            dfScaleSrcMin = adfCMinMax[0];
            dfScaleSrcMax = adfCMinMax[1];
        }

        if( bScale )
        {
            if( dfScaleSrcMax == dfScaleSrcMin )
                dfScaleSrcMax += 0.1;
            if( dfScaleDstMax == dfScaleDstMin )
                dfScaleDstMax += 0.1;

            dfScale = (dfScaleDstMax - dfScaleDstMin)
                    / (dfScaleSrcMax - dfScaleSrcMin);
            dfOffset = -1 * dfScaleSrcMin * dfScale + dfScaleDstMin;
        }

        if( bUnscale )
        {
            dfScale = poSrcBand->GetScale();
            dfOffset = poSrcBand->GetOffset();
        }

        /* -------------------------------------------------------------------- */
        /*      Create a simple or complex data source depending on the         */
        /*      translation type required.                                      */
        /* -------------------------------------------------------------------- */
        if( bUnscale || bScale || (nRGBExpand != 0 && i < nRGBExpand) )
        {
            poVRTBand->AddComplexSource( poSrcBand,
                                         anSrcWin[0], anSrcWin[1],
                    anSrcWin[2], anSrcWin[3],
                    anDstWin[0], anDstWin[1],
                    anDstWin[2], anDstWin[3],
                    dfOffset, dfScale,
                    VRT_NODATA_UNSET,
                    nComponent );
        }
        else
            poVRTBand->AddSimpleSource( poSrcBand,
                                        anSrcWin[0], anSrcWin[1],
                    anSrcWin[2], anSrcWin[3],
                    anDstWin[0], anDstWin[1],
                    anDstWin[2], anDstWin[3] );

        /* -------------------------------------------------------------------- */
        /*      In case of color table translate, we only set the color         */
        /*      interpretation other info copied by CopyBandInfo are            */
        /*      not relevant in RGB expansion.                                  */
        /* -------------------------------------------------------------------- */
        if (nRGBExpand == 1)
        {
            poVRTBand->SetColorInterpretation( GCI_GrayIndex );
        }
        else if (nRGBExpand != 0 && i < nRGBExpand)
        {
            poVRTBand->SetColorInterpretation( (GDALColorInterp) (GCI_RedBand + i) );
        }

        /* -------------------------------------------------------------------- */
        /*      copy over some other information of interest.                   */
        /* -------------------------------------------------------------------- */
        else
        {
            CopyBandInfo( poSrcBand, poVRTBand,
                          !bStats && !bFilterOutStatsMetadata,
                          !bUnscale,
                          !bSetNoData && !bUnsetNoData );
        }

        /* -------------------------------------------------------------------- */
        /*      Set a forcable nodata value?                                    */
        /* -------------------------------------------------------------------- */
        if( bSetNoData )
        {
            double dfVal = dfNoDataReal;
            int bClamped = FALSE, bRounded = FALSE;

#define CLAMP(val,type,minval,maxval) \
    do { if (val < minval) { bClamped = TRUE; val = minval; } \
    else if (val > maxval) { bClamped = TRUE; val = maxval; } \
    else if (val != (type)val) { bRounded = TRUE; val = (type)(val + 0.5); } } \
    while(0)

            switch(eBandType)
            {
            case GDT_Byte:
                CLAMP(dfVal, GByte, 0.0, 255.0);
                break;
            case GDT_Int16:
                CLAMP(dfVal, GInt16, -32768.0, 32767.0);
                break;
            case GDT_UInt16:
                CLAMP(dfVal, GUInt16, 0.0, 65535.0);
                break;
            case GDT_Int32:
                CLAMP(dfVal, GInt32, -2147483648.0, 2147483647.0);
                break;
            case GDT_UInt32:
                CLAMP(dfVal, GUInt32, 0.0, 4294967295.0);
                break;
            default:
                break;
            }

            if (bClamped)
            {
                printf( "for band %d, nodata value has been clamped "
                        "to %.0f, the original value being out of range.\n",
                        i + 1, dfVal);
            }
            else if(bRounded)
            {
                printf("for band %d, nodata value has been rounded "
                       "to %.0f, %s being an integer datatype.\n",
                       i + 1, dfVal,
                       GDALGetDataTypeName(eBandType));
            }

            poVRTBand->SetNoDataValue( dfVal );
        }

        if (eMaskMode == MASK_AUTO &&
                (GDALGetMaskFlags(GDALGetRasterBand(hDataset, 1)) & GMF_PER_DATASET) == 0 &&
                (poSrcBand->GetMaskFlags() & (GMF_ALL_VALID | GMF_NODATA)) == 0)
        {
            if (poVRTBand->CreateMaskBand(poSrcBand->GetMaskFlags()) == CE_None)
            {
                VRTSourcedRasterBand* hMaskVRTBand =
                        (VRTSourcedRasterBand*)poVRTBand->GetMaskBand();
                hMaskVRTBand->AddMaskBandSource(poSrcBand,
                                                anSrcWin[0], anSrcWin[1],
                        anSrcWin[2], anSrcWin[3],
                        anDstWin[0], anDstWin[1],
                        anDstWin[2], anDstWin[3] );
            }
        }
    }

    if (eMaskMode == MASK_USER)
    {
        GDALRasterBand *poSrcBand =
                (GDALRasterBand*)GDALGetRasterBand(hDataset, ABS(nMaskBand));
        if (poSrcBand && poVDS->CreateMaskBand(GMF_PER_DATASET) == CE_None)
        {
            VRTSourcedRasterBand* hMaskVRTBand = (VRTSourcedRasterBand*)
                    GDALGetMaskBand(GDALGetRasterBand((GDALDatasetH)poVDS, 1));
            if (nMaskBand > 0)
                hMaskVRTBand->AddSimpleSource(poSrcBand,
                                              anSrcWin[0], anSrcWin[1],
                        anSrcWin[2], anSrcWin[3],
                        anDstWin[0], anDstWin[1],
                        anDstWin[2], anDstWin[3] );
            else
                hMaskVRTBand->AddMaskBandSource(poSrcBand,
                                                anSrcWin[0], anSrcWin[1],
                        anSrcWin[2], anSrcWin[3],
                        anDstWin[0], anDstWin[1],
                        anDstWin[2], anDstWin[3] );
        }
    }
    else
        if (eMaskMode == MASK_AUTO && nSrcBandCount > 0 &&
                GDALGetMaskFlags(GDALGetRasterBand(hDataset, 1)) == GMF_PER_DATASET)
        {
            if (poVDS->CreateMaskBand(GMF_PER_DATASET) == CE_None)
            {
                VRTSourcedRasterBand* hMaskVRTBand = (VRTSourcedRasterBand*)
                        GDALGetMaskBand(GDALGetRasterBand((GDALDatasetH)poVDS, 1));
                hMaskVRTBand->AddMaskBandSource((GDALRasterBand*)GDALGetRasterBand(hDataset, 1),
                                                anSrcWin[0], anSrcWin[1],
                        anSrcWin[2], anSrcWin[3],
                        anDstWin[0], anDstWin[1],
                        anDstWin[2], anDstWin[3] );
            }
        }

    /* -------------------------------------------------------------------- */
    /*      Compute stats if required.                                      */
    /* -------------------------------------------------------------------- */
    if (bStats)
    {
        for( i = 0; i < poVDS->GetRasterCount(); i++ )
        {
            double dfMin, dfMax, dfMean, dfStdDev;
            poVDS->GetRasterBand(i+1)->ComputeStatistics( bApproxStats,
                                                          &dfMin, &dfMax, &dfMean, &dfStdDev, GDALDummyProgress, NULL );
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Write to the output file using CopyCreate().                    */
    /* -------------------------------------------------------------------- */
    hOutDS = GDALCreateCopy( hDriver, pszDest, (GDALDatasetH) poVDS,
                             bStrict, papszCreateOptions,
                             pfnProgress, NULL );
    if( hOutDS != NULL )
    {
        int bHasGotErr = FALSE;
        CPLErrorReset();
        GDALFlushCache( hOutDS );
        if (CPLGetLastErrorType() != CE_None)
            bHasGotErr = TRUE;
        GDALClose( hOutDS );
        if (bHasGotErr)
            hOutDS = NULL;
    }

    GDALClose( (GDALDatasetH) poVDS );

    GDALClose( hDataset );

    CPLFree( panBandList );

    CPLFree( pszOutputSRS );

    if( !bSubCall )
    {
        GDALDumpOpenDatasets( stderr );
        GDALDestroyDriverManager();
    }

    CSLDestroy( argv );
    CSLDestroy( papszCreateOptions );

    return hOutDS == NULL;
}

} // namespace nav_tools

