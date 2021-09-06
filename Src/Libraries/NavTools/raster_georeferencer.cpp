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

static int argIsNumeric(const char * _psz_arg);
static void attachMetadata( GDALDatasetH, char ** );
static void copyBandInfo( GDALRasterBand * _po_src_band, GDALRasterBand * _po_dst_band,
                          int _b_can_copy_stats_metadata, int _b_copy_scale, int _b_copy_no_data );
static int b_sub_call = FALSE;

/*  ******************************************************************* */
/*           Kept usage from old command line gdal_translate            */
/* ******************************************************************** */

static void usage(const char* _psz_error_msg = NULL, int _b_short = TRUE)

{
    int	i_dr;

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

    if( !_b_short )
    {
        printf( "\n%s\n\n", GDALVersionInfo( "--version" ) );
        printf( "The following format drivers are configured and support output:\n" );
        for( i_dr = 0; i_dr < GDALGetDriverCount(); i_dr++ )
        {
            GDALDriverH h_driver = GDALGetDriver(i_dr);

            if( GDALGetMetadataItem( h_driver, GDAL_DCAP_CREATE, NULL ) != NULL
                    || GDALGetMetadataItem( h_driver, GDAL_DCAP_CREATECOPY,
                                            NULL ) != NULL )
            {
                printf( "  %s: %s\n",
                        GDALGetDriverShortName( h_driver ),
                        GDALGetDriverLongName( h_driver ) );
            }
        }
    }

    if( _psz_error_msg != NULL )
        fprintf(stderr, "\nFAILURE: %s\n", _psz_error_msg);

    exit(1);
}

/* -------------------------------------------------------------------- */
/*                      checkExtensionConsistency()                     */
/*                                                                      */
/*      Check that the target file extension is consistant with the     */
/*      requested driver. Actually, we only warn in cases where the     */
/*      inconsistency is blatant (use of an extension declared by one   */
/*      or several drivers, and not by the selected one)                */
/* -------------------------------------------------------------------- */


void checkExtensionConsistency(const char* _psz_dest_filename,
                               const char* _psz_driver_name)
{

    char* psz_dest_extension = CPLStrdup(CPLGetExtension(_psz_dest_filename));
    if (psz_dest_extension[0] != '\0')
    {
        int n_driver_count = GDALGetDriverCount();
        CPLString os_conflicting_driver_list;
        for(int i=0;i<n_driver_count;i++)
        {
            GDALDriverH h_driver = GDALGetDriver(i);
            const char* psz_driver_extension =
                    GDALGetMetadataItem( h_driver, GDAL_DMD_EXTENSION, NULL );
            if (psz_driver_extension && EQUAL(psz_dest_extension, psz_driver_extension))
            {
                if (GDALGetDriverByName(_psz_driver_name) != h_driver)
                {
                    if (os_conflicting_driver_list.size())
                        os_conflicting_driver_list += ", ";
                    os_conflicting_driver_list += GDALGetDriverShortName(h_driver);
                }
                else
                {
                    /* If the request driver allows the used extension, then */
                    /* just stop iterating now */
                    os_conflicting_driver_list = "";
                    break;
                }
            }
        }
        if (os_conflicting_driver_list.size())
        {
            fprintf(stderr,
                    "Warning: The target file has a '%s' extension, which is normally used by the %s driver%s,\n"
                    "but the requested output driver is %s. Is it really what you want ?\n",
                    psz_dest_extension,
                    os_conflicting_driver_list.c_str(),
                    strchr(os_conflicting_driver_list.c_str(), ',') ? "s" : "",
                    _psz_driver_name);
        }
    }

    CPLFree(psz_dest_extension);
}

/* -------------------------------------------------------------------- */
/*                        earlySetConfigOptions()                       */
/* -------------------------------------------------------------------- */

void earlySetConfigOptions( int _argc, char ** _argv )
{
    /* Must process some config options before GDALAllRegister() or OGRRegisterAll(), */
    /* but we can't call GDALGeneralCmdLineProcessor() or OGRGeneralCmdLineProcessor(), */
    /* because it needs the drivers to be registered for the --format or --formats options */
    for( int i = 1; i < _argc; i++ )
    {
        if( EQUAL(_argv[i],"--config") && i + 2 < _argc &&
                (EQUAL(_argv[i + 1], "GDAL_SKIP") ||
                 EQUAL(_argv[i + 1], "GDAL_DRIVER_PATH") ||
                 EQUAL(_argv[i + 1], "OGR_SKIP") ||
                 EQUAL(_argv[i + 1], "OGR_DRIVER_PATH")) )
        {
            CPLSetConfigOption( _argv[i+1], _argv[i+2] );

            i += 2;
        }
    }
}


/************************************************************************/
/*                              srcToDst()                              */
/************************************************************************/

static void srcToDst( double _df_x, double _df_y,
                      int _n_src_x_off, int _n_src_y_off,
                      int _n_src_x_size, int _n_src_y_size,
                      int _n_dst_x_off, int _n_dst_y_Off,
                      int _n_dst_x_size, int _n_dst_y_size,
                      double &_df_x_out, double &_df_y_out )

{
    _df_x_out = ((_df_x - _n_src_x_off) / _n_src_x_size) * _n_dst_x_size + _n_dst_x_off;
    _df_y_out = ((_df_y - _n_src_y_off) / _n_src_y_size) * _n_dst_y_size + _n_dst_y_Off;
}

/************************************************************************/
/*                          fixSrcDstWindow()                           */
/************************************************************************/

static int fixSrcDstWindow( int* _pan_src_win, int* _pan_dst_win,
                            int _n_src_raster_x_size,
                            int _n_src_raster_y_size )

{
    const int n_src_x_off = _pan_src_win[0];
    const int n_src_y_off = _pan_src_win[1];
    const int n_src_x_size = _pan_src_win[2];
    const int n_src_y_size = _pan_src_win[3];

    const int n_dst_x_off = _pan_dst_win[0];
    const int n_dst_y_off = _pan_dst_win[1];
    const int n_dst_x_size = _pan_dst_win[2];
    const int n_dst_y_size = _pan_dst_win[3];

    int b_modified_x = FALSE, b_modified_y = FALSE;

    int n_modified_src_x_off = n_src_x_off;
    int n_modified_src_y_off = n_src_y_off;

    int n_modified_src_x_size = n_src_x_size;
    int n_modified_src_y_size = n_src_y_size;

    /* -------------------------------------------------------------------- */
    /*      Clamp within the bounds of the available source data.           */
    /* -------------------------------------------------------------------- */
    if( n_modified_src_x_off < 0 )
    {
        n_modified_src_x_size += n_modified_src_x_off;
        n_modified_src_x_off = 0;

        b_modified_x = TRUE;
    }

    if( n_modified_src_y_off < 0 )
    {
        n_modified_src_y_size += n_modified_src_y_off;
        n_modified_src_y_off = 0;
        b_modified_y = TRUE;
    }

    if( n_modified_src_x_off + n_modified_src_x_size > _n_src_raster_x_size )
    {
        n_modified_src_x_size = _n_src_raster_x_size - n_modified_src_x_off;
        b_modified_x = TRUE;
    }

    if( n_modified_src_y_off + n_modified_src_y_size > _n_src_raster_y_size )
    {
        n_modified_src_y_size = _n_src_raster_y_size - n_modified_src_y_off;
        b_modified_y = TRUE;
    }

    /* -------------------------------------------------------------------- */
    /*      Don't do anything if the requesting region is completely off    */
    /*      the source image.                                               */
    /* -------------------------------------------------------------------- */
    if( n_modified_src_x_off >= _n_src_raster_x_size
            || n_modified_src_y_off >= _n_src_raster_y_size
            || n_modified_src_x_size <= 0 || n_modified_src_y_size <= 0 )
    {
        return FALSE;
    }

    _pan_src_win[0] = n_modified_src_x_off;
    _pan_src_win[1] = n_modified_src_y_off;
    _pan_src_win[2] = n_modified_src_x_size;
    _pan_src_win[3] = n_modified_src_y_size;

    /* -------------------------------------------------------------------- */
    /*      If we haven't had to modify the source rectangle, then the      */
    /*      destination rectangle must be the whole region.                 */
    /* -------------------------------------------------------------------- */
    if( !b_modified_x && !b_modified_y )
        return TRUE;

    /* -------------------------------------------------------------------- */
    /*      Now transform this possibly reduced request back into the       */
    /*      destination buffer coordinates in case the output region is     */
    /*      less than the whole buffer.                                     */
    /* -------------------------------------------------------------------- */
    double df_dst_ulx, df_dst_uly, df_dst_lrx, df_dst_lry;

    srcToDst( n_modified_src_x_off, n_modified_src_y_off,
              n_src_x_off, n_src_y_off,
              n_src_x_size, n_src_y_size,
              n_dst_x_off, n_dst_y_off,
              n_dst_x_size, n_dst_y_size,
              df_dst_ulx, df_dst_uly );
    srcToDst( n_modified_src_x_off + n_modified_src_x_size, n_modified_src_y_off + n_modified_src_y_size,
              n_src_x_off, n_src_y_off,
              n_src_x_size, n_src_y_size,
              n_dst_x_off, n_dst_y_off,
              n_dst_x_size, n_dst_y_size,
              df_dst_lrx, df_dst_lry );

    int n_modified_dst_x_off = n_dst_x_off;
    int n_modified_dst_y_off = n_dst_y_off;
    int n_modified_dst_x_size = n_dst_x_size;
    int n_modified_dst_y_size = n_dst_y_size;

    if( b_modified_x )
    {
        n_modified_dst_x_off = (int) ((df_dst_ulx - n_dst_x_off)+0.001);
        n_modified_dst_x_size = (int) ((df_dst_lrx - n_dst_x_off)+0.001)
                - n_modified_dst_x_off;

        n_modified_dst_x_off = MAX(0,n_modified_dst_x_off);
        if( n_modified_dst_x_off + n_modified_dst_x_size > n_dst_x_size )
            n_modified_dst_x_size = n_dst_x_size - n_modified_dst_x_off;
    }

    if( b_modified_y )
    {
        n_modified_dst_y_off = (int) ((df_dst_uly - n_dst_y_off)+0.001);
        n_modified_dst_y_size = (int) ((df_dst_lry - n_dst_y_off)+0.001)
                - n_modified_dst_y_off;

        n_modified_dst_y_off = MAX(0,n_modified_dst_y_off);
        if( n_modified_dst_y_off + n_modified_dst_y_size > n_dst_y_size )
            n_modified_dst_y_size = n_dst_y_size - n_modified_dst_y_off;
    }

    if( n_modified_dst_x_size < 1 || n_modified_dst_y_size < 1 )
        return FALSE;
    else
    {
        _pan_dst_win[0] = n_modified_dst_x_off;
        _pan_dst_win[1] = n_modified_dst_y_off;
        _pan_dst_win[2] = n_modified_dst_x_size;
        _pan_dst_win[3] = n_modified_dst_y_size;

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
    usage(CPLSPrintf("%s option requires %d argument(s)", argv[i], nExtraArg)); } while(0)




/************************************************************************/
/*                            argIsNumeric()                            */
/************************************************************************/

int argIsNumeric( const char *_psz_arg )

{
    return CPLGetValueType(_psz_arg) != CPL_VALUE_STRING;
}

/************************************************************************/
/*                           attachMetadata()                           */
/************************************************************************/

static void attachMetadata( GDALDatasetH _h_ds, char **_papsz_metadata_options )

{
    int n_count = CSLCount(_papsz_metadata_options);
    int i;

    for( i = 0; i < n_count; i++ )
    {
        char    *psz_key = NULL;
        const char *psz_value;

        psz_value = CPLParseNameValue( _papsz_metadata_options[i], &psz_key );
        GDALSetMetadataItem(_h_ds,psz_key,psz_value,NULL);
        CPLFree( psz_key );
    }

    CSLDestroy( _papsz_metadata_options );
}

/************************************************************************/
/*                           copyBandInfo()                            */
/************************************************************************/

/* A bit of a clone of VRTRasterBand::CopyCommonInfoFrom(), but we need */
/* more and more custom behaviour in the context of gdal_translate ... */

static void copyBandInfo(GDALRasterBand * _po_src_band, GDALRasterBand * _po_dst_band,
                          int _b_can_copy_stats_metadata, int _b_copy_scale, int _b_copy_no_data )

{
    int b_success;
    double df_no_data;

    if (_b_can_copy_stats_metadata)
    {
        _po_dst_band->SetMetadata( _po_src_band->GetMetadata() );
    }
    else
    {
        char** papsz_metadata = _po_src_band->GetMetadata();
        char** papsz_metadata_new = NULL;
        for( int i = 0; papsz_metadata != NULL && papsz_metadata[i] != NULL; i++ )
        {
            if (strncmp(papsz_metadata[i], "STATISTICS_", 11) != 0)
                papsz_metadata_new = CSLAddString(papsz_metadata_new, papsz_metadata[i]);
        }
        _po_dst_band->SetMetadata( papsz_metadata_new );
        CSLDestroy(papsz_metadata_new);
    }

    _po_dst_band->SetColorTable( _po_src_band->GetColorTable() );
    _po_dst_band->SetColorInterpretation(_po_src_band->GetColorInterpretation());
    if( strlen(_po_src_band->GetDescription()) > 0 )
        _po_dst_band->SetDescription( _po_src_band->GetDescription() );

    if (_b_copy_no_data)
    {
        df_no_data = _po_src_band->GetNoDataValue( &b_success );
        if( b_success )
            _po_dst_band->SetNoDataValue( df_no_data );
    }

    if (_b_copy_scale)
    {
        _po_dst_band->SetOffset( _po_src_band->GetOffset() );
        _po_dst_band->SetScale( _po_src_band->GetScale() );
    }

    _po_dst_band->SetCategoryNames( _po_src_band->GetCategoryNames() );
    if( !EQUAL(_po_src_band->GetUnitType(),"") )
        _po_dst_band->SetUnitType( _po_src_band->GetUnitType() );

}


// Beginning of class implementation ************************************************************************

RasterGeoreferencer::RasterGeoreferencer():m_psz_format("GTiff")
{
    m_b_format_explicitely_set = FALSE;
    m_pan_band_list = NULL; /* negative value of panBandList[i] means mask band of ABS(panBandList[i]) */
    m_n_band_count = 0;
    m_b_def_bands = TRUE;
    m_adf_geo_transform = (double*)malloc(6*sizeof(double));
    m_e_output_type = GDT_Unknown;
    m_n_ox_size = 0;
    m_n_oy_size = 0;
    m_psz_ox_size=NULL;
    m_psz_oy_size=NULL;
    m_papsz_create_options = NULL;
    m_an_src_win = (int*)malloc(4*sizeof(int));
    m_b_strict = FALSE;
    m_b_scale = FALSE;
    m_b_have_scale_src = FALSE;
    m_b_unscale=FALSE;
    m_df_scale_src_min=0.0;
    m_df_scale_src_max=255.0;
    m_df_scale_dst_min=0.0;
    m_df_scale_dst_max=255.0;
    m_papsz_metadata_options = NULL;
    m_psz_output_srs = NULL;
    m_b_quiet = FALSE;
    m_b_got_bounds = FALSE;
    m_pfn_progress = GDALTermProgress;
    m_n_gcp_count = 0;
    m_pas_gcps = NULL;
    m_i_src_file_arg = -1;
    m_i_dst_file_arg = -1;
    b_copy_sub_datasets = FALSE;
    m_adf_ullr = (double*)malloc(4*sizeof(double));
    m_adf_ullr[0]=0; m_adf_ullr[1]=0; m_adf_ullr[2]=0; m_adf_ullr[3]=0;
    m_b_set_no_data = FALSE;
    m_b_unset_no_data = FALSE;
    m_df_no_data_real = 0.0;
    m_n_rgb_expand = 0;
    m_b_parsed_mask_argument = FALSE;
    m_e_mask_mode = MASK_AUTO;
    m_n_mask_band = 0; /* negative value means mask band of ABS(nMaskBand) */
    m_b_stats = FALSE, m_b_approx_stats = FALSE;
    m_b_error_on_partially_outside = FALSE;
    m_b_error_on_completely_outside = FALSE;
    m_psz_dest = NULL;

}

RasterGeoreferencer::~RasterGeoreferencer()
{
    free(m_adf_geo_transform);
    free(m_an_src_win);
    free(m_adf_ullr);
}

int RasterGeoreferencer::writeGeoFile(Mat &_raster, Mat &_raster_mask, QString _output_file, QString _cmd_line_options)
{



    // Complete arguments for exe name and mask
    QString local_argv;
    if(_raster.channels()>1){
        local_argv= QString("WriteGeoFile ") + _cmd_line_options + QString(" -mask 4 --config GDAL_TIFF_INTERNAL_MASK YES ") + _output_file;
    }else{
        local_argv= QString("WriteGeoFile ") + _cmd_line_options + QString(" -mask 2 --config GDAL_TIFF_INTERNAL_MASK YES ") + _output_file;
    }

    fprintf(stderr,"%s\n", local_argv.toStdString().c_str());

    // recreate argc and argv for options parsing **************************************

    QStringList cmd_quote_split_list = local_argv.split("\"");
    QStringList argv_list;

    int i=1;

    // isolate quote separated argument as a unique argument and split others on space
    foreach(QString arguments, cmd_quote_split_list){

        if (i%2 == 0){
            argv_list += arguments;
        }else{
            argv_list += arguments.split(" ",QString::SkipEmptyParts);
        }
        i++;
    }
    argv_list.removeAll(" ");

    int argc = argv_list.length();
    char **argv;
    argv = (char **)malloc(argc*sizeof(char*));

    i=0;
    foreach(QString argument, argv_list){

        argv[i] = (char*) malloc(sizeof(char)*argument.size());
        strcpy ( argv[i], argument.toStdString().c_str() );
        i++;
    }

    // *********************************************************************************

    m_an_src_win[0] = 0;
    m_an_src_win[1] = 0;
    m_an_src_win[2] = 0;
    m_an_src_win[3] = 0;

    m_df_ulx = m_df_uly = m_df_lrx = m_df_lry = 0.0;

    /* Check strict compilation and runtime library version as we use C++ API */
    if (! GDAL_CHECK_VERSION(argv[0]))
        exit(1);

    earlySetConfigOptions(argc, argv);

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
            usage();
        else if ( EQUAL(argv[i], "--long-usage") )
        {
            usage(NULL, FALSE);
        }
        else if( EQUAL(argv[i],"-of") && i < argc-1 )
        {
            m_psz_format = argv[++i];
            m_b_format_explicitely_set = TRUE;
        }

        else if( EQUAL(argv[i],"-q") || EQUAL(argv[i],"-quiet") )
        {
            m_b_quiet = TRUE;
            m_pfn_progress = GDALDummyProgress;
        }

        else if( EQUAL(argv[i],"-ot") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            int	i_type;

            for( i_type = 1; i_type < GDT_TypeCount; i_type++ )
            {
                if( GDALGetDataTypeName((GDALDataType)i_type) != NULL
                        && EQUAL(GDALGetDataTypeName((GDALDataType)i_type),
                                 argv[i+1]) )
                {
                    m_e_output_type = (GDALDataType) i_type;
                }
            }

            if( m_e_output_type == GDT_Unknown )
            {
                usage(CPLSPrintf("Unknown output pixel type: %s.", argv[i+1] ));
            }
            i++;
        }
        else if( EQUAL(argv[i],"-b") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            const char* psz_band = argv[i+1];
            int bMask = FALSE;
            if (EQUAL(psz_band, "mask"))
                psz_band = "mask,1";
            if (EQUALN(psz_band, "mask,", 5))
            {
                bMask = TRUE;
                psz_band += 5;
                /* If we use tha source mask band as a regular band */
                /* don't create a target mask band by default */
                if( !m_b_parsed_mask_argument )
                    m_e_mask_mode = MASK_DISABLED;
            }
            int n_band = atoi(psz_band);
            if( n_band < 1 )
            {
                usage(CPLSPrintf( "Unrecognizable band number (%s).", argv[i+1] ));
            }
            i++;

            m_n_band_count++;
            m_pan_band_list = (int *)
                    CPLRealloc(m_pan_band_list, sizeof(int) * m_n_band_count);
            m_pan_band_list[m_n_band_count-1] = n_band;
            if (bMask)
                m_pan_band_list[m_n_band_count-1] *= -1;

            if( m_pan_band_list[m_n_band_count-1] != m_n_band_count )
                m_b_def_bands = FALSE;
        }
        else if( EQUAL(argv[i],"-mask") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            m_b_parsed_mask_argument = TRUE;
            const char* psz_band = argv[i+1];
            if (EQUAL(psz_band, "none"))
            {
                m_e_mask_mode = MASK_DISABLED;
            }
            else if (EQUAL(psz_band, "auto"))
            {
                m_e_mask_mode = MASK_AUTO;
            }
            else
            {
                int b_mask = FALSE;
                if (EQUAL(psz_band, "mask"))
                    psz_band = "mask,1";
                if (EQUALN(psz_band, "mask,", 5))
                {
                    b_mask = TRUE;
                    psz_band += 5;
                }
                int n_band = atoi(psz_band);
                if( n_band < 1 )
                {
                    usage(CPLSPrintf( "Unrecognizable band number (%s).", argv[i+1] ));
                }

                m_e_mask_mode = MASK_USER;
                m_n_mask_band = n_band;
                if (b_mask)
                    m_n_mask_band *= -1;
            }
            i ++;
        }
        else if( EQUAL(argv[i],"-not_strict")  )
            m_b_strict = FALSE;

        else if( EQUAL(argv[i],"-strict")  )
            m_b_strict = TRUE;

        else if( EQUAL(argv[i],"-sds")  )
            b_copy_sub_datasets = TRUE;

        else if( EQUAL(argv[i],"-gcp") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(4);
            char* endptr = NULL;
            /* -gcp pixel line easting northing [elev] */

            m_n_gcp_count++;
            m_pas_gcps = (GDAL_GCP *)
                    CPLRealloc( m_pas_gcps, sizeof(GDAL_GCP) * m_n_gcp_count );
            GDALInitGCPs( 1, m_pas_gcps + m_n_gcp_count - 1 );

            m_pas_gcps[m_n_gcp_count-1].dfGCPPixel = CPLAtofM(argv[++i]);
            m_pas_gcps[m_n_gcp_count-1].dfGCPLine = CPLAtofM(argv[++i]);
            m_pas_gcps[m_n_gcp_count-1].dfGCPX = CPLAtofM(argv[++i]);
            m_pas_gcps[m_n_gcp_count-1].dfGCPY = CPLAtofM(argv[++i]);
            if( argv[i+1] != NULL
                    && (CPLStrtod(argv[i+1], &endptr) != 0.0 || argv[i+1][0] == '0') )
            {
                /* Check that last argument is really a number and not a filename */
                /* looking like a number (see ticket #863) */
                if (endptr && *endptr == 0)
                    m_pas_gcps[m_n_gcp_count-1].dfGCPZ = CPLAtofM(argv[++i]);
            }

            /* should set id and info? */
        }

        else if( EQUAL(argv[i],"-a_nodata") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            if (EQUAL(argv[i+1], "none"))
            {
                m_b_unset_no_data = TRUE;
            }
            else
            {
                m_b_set_no_data = TRUE;
                m_df_no_data_real = CPLAtofM(argv[i+1]);
            }
            i += 1;
        }

        else if( EQUAL(argv[i],"-a_ullr") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(4);
            m_adf_ullr[0] = CPLAtofM(argv[i+1]);
            m_adf_ullr[1] = CPLAtofM(argv[i+2]);
            m_adf_ullr[2] = CPLAtofM(argv[i+3]);
            m_adf_ullr[3] = CPLAtofM(argv[i+4]);

            m_b_got_bounds = TRUE;

            i += 4;
        }

        else if( EQUAL(argv[i],"-co") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            m_papsz_create_options = CSLAddString( m_papsz_create_options, argv[++i] );
        }

        else if( EQUAL(argv[i],"-scale") )
        {
            m_b_scale = TRUE;
            if( i < argc-2 && argIsNumeric(argv[i+1]) )
            {
                m_b_have_scale_src = TRUE;
                m_df_scale_src_min = CPLAtofM(argv[i+1]);
                m_df_scale_src_max = CPLAtofM(argv[i+2]);
                i += 2;
            }
            if( i < argc-2 && m_b_have_scale_src && argIsNumeric(argv[i+1]) )
            {
                m_df_scale_dst_min = CPLAtofM(argv[i+1]);
                m_df_scale_dst_max = CPLAtofM(argv[i+2]);
                i += 2;
            }
            else
            {
                m_df_scale_dst_min = 0.0;
                m_df_scale_dst_max = 255.999;
            }
        }

        else if( EQUAL(argv[i], "-unscale") )
        {
            m_b_unscale = TRUE;
        }

        else if( EQUAL(argv[i],"-mo") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            m_papsz_metadata_options = CSLAddString( m_papsz_metadata_options,
                                                 argv[++i] );
        }

        else if( EQUAL(argv[i],"-outsize") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(2);
            m_psz_ox_size = argv[++i];
            m_psz_oy_size = argv[++i];
        }

        else if( EQUAL(argv[i],"-srcwin") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(4);
            m_an_src_win[0] = atoi(argv[++i]);
            m_an_src_win[1] = atoi(argv[++i]);
            m_an_src_win[2] = atoi(argv[++i]);
            m_an_src_win[3] = atoi(argv[++i]);
        }

        else if( EQUAL(argv[i],"-projwin") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(4);
            m_df_ulx = CPLAtofM(argv[++i]);
            m_df_uly = CPLAtofM(argv[++i]);
            m_df_lrx = CPLAtofM(argv[++i]);
            m_df_lry = CPLAtofM(argv[++i]);
        }

        else if( EQUAL(argv[i],"-epo") )
        {
            m_b_error_on_partially_outside = TRUE;
            m_b_error_on_completely_outside = TRUE;
        }

        else  if( EQUAL(argv[i],"-eco") )
        {
            m_b_error_on_completely_outside = TRUE;
        }

        else if( EQUAL(argv[i],"-a_srs") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            OGRSpatialReference o_output_srs;

            if( o_output_srs.SetFromUserInput( argv[i+1] ) != OGRERR_NONE )
            {
                fprintf( stderr, "Failed to process SRS definition: %s\n",
                         argv[i+1] );
                GDALDestroyDriverManager();
                exit( 1 );
            }

            o_output_srs.exportToWkt( &m_psz_output_srs );
            i++;
        }

        else if( EQUAL(argv[i],"-expand") )
        {
            CHECK_HAS_ENOUGH_ADDITIONAL_ARGS(1);
            if (EQUAL(argv[i+1], "gray"))
                m_n_rgb_expand = 1;
            else if (EQUAL(argv[i+1], "rgb"))
                m_n_rgb_expand = 3;
            else if (EQUAL(argv[i+1], "rgba"))
                m_n_rgb_expand = 4;
            else
            {
                usage(CPLSPrintf( "Value %s unsupported. Only gray, rgb or rgba are supported.",
                                  argv[i] ));
            }
            i++;
        }

        else if( EQUAL(argv[i], "-stats") )
        {
            m_b_stats = TRUE;
            m_b_approx_stats = FALSE;
        }
        else if( EQUAL(argv[i], "-approx_stats") )
        {
            m_b_stats = TRUE;
            m_b_approx_stats = TRUE;
        }

        else if( argv[i][0] == '-' )
        {
            usage(CPLSPrintf("Unkown option name '%s'", argv[i]));
        }
        else if( m_psz_dest == NULL )
        {
            m_psz_dest = argv[i];
            m_i_dst_file_arg = i;
            printf( "pszDest = %s \n", m_psz_dest );
        }

        else
        {
            usage("Too many command options.");
        }
    }

    if( m_psz_dest == NULL )
    {
        usage("No target dataset specified.");
    }

    if( strcmp(m_psz_dest, "/vsistdout/") == 0)
    {
        m_b_quiet = TRUE;
        m_pfn_progress = GDALDummyProgress;
    }

    if (!m_b_quiet && !m_b_format_explicitely_set)
        checkExtensionConsistency(m_psz_dest, m_psz_format);

    /* -------------------------------------------------------------------- */
    /*      Attempt to open source file.                                    */
    /* -------------------------------------------------------------------- */

    cout <<  "Nb of registered drivers = " << GetGDALDriverManager()->GetDriverCount() << std::endl ;
    m_input_driver = GetGDALDriverManager()->GetDriverByName("MEM");

    if(! m_input_driver)                              // Check for invalid input
    {
        cout <<  "Could not create the input driver" << std::endl ;
        return -1;
    }


    if(! _raster.data || ! _raster_mask.data)                              // Check for invalid input
    {
        cout <<  "Raster data is incomplete" << std::endl ;
        return -1;
    }


    GDALRasterBand *p_band;
    if (_raster.channels()>1){
        m_po_dataset = m_input_driver->Create ("", _raster.cols, _raster.rows, 4, GDT_Byte, NULL);
    }else{
        m_po_dataset = m_input_driver->Create ("", _raster.cols, _raster.rows, 2, GDT_Byte, NULL);
    }

    //Copy raster data & mask
    if (_raster.channels()>1){

        vector<Mat> rasterChannels(3);
        split(_raster, rasterChannels);

        for(int k=0;k<3;k++){
            p_band = m_po_dataset->GetRasterBand(3-k);
            p_band->RasterIO(GF_Write,0,0,_raster.cols,_raster.rows,rasterChannels[k].data,_raster.cols,_raster.rows,GDT_Byte,0,0);
        }

        p_band = m_po_dataset->GetRasterBand(4);
        p_band->RasterIO(GF_Write,0,0,_raster_mask.cols,_raster_mask.rows,_raster_mask.data,_raster.cols,_raster.rows,GDT_Byte,0,0);

    }else{
        p_band = m_po_dataset->GetRasterBand(1);
        p_band->RasterIO(GF_Write,0,0,_raster.cols,_raster.rows,_raster.data,_raster.cols,_raster.rows,GDT_Byte,0,0);

        p_band = m_po_dataset->GetRasterBand(2);
        p_band->RasterIO(GF_Write,0,0,_raster.cols,_raster.rows,_raster_mask.data,_raster.cols,_raster.rows,GDT_Byte,0,0);
    }



    m_h_dataset = m_po_dataset; // C++ -> C casting for retrocompat

    if( m_h_dataset == NULL )
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
    m_n_raster_x_size = GDALGetRasterXSize( m_h_dataset );
    m_n_raster_y_size = GDALGetRasterYSize( m_h_dataset );

    if( !m_b_quiet )
        printf( "Input file size is %d, %d\n", m_n_raster_x_size, m_n_raster_y_size );

    if( m_an_src_win[2] == 0 && m_an_src_win[3] == 0 )
    {
        m_an_src_win[2] = m_n_raster_x_size;
        m_an_src_win[3] = m_n_raster_y_size;
    }

    /* -------------------------------------------------------------------- */
    /*	Build band list to translate					*/
    /* -------------------------------------------------------------------- */
    if( m_n_band_count == 0 )
    {
        m_n_band_count = GDALGetRasterCount( m_h_dataset );
        if( m_n_band_count == 0 )
        {
            fprintf( stderr, "Input file has no bands, and so cannot be translated.\n" );
            GDALDestroyDriverManager();
            exit(1 );
        }

        m_pan_band_list = (int *) CPLMalloc(sizeof(int)*m_n_band_count);
        for( i = 0; i < m_n_band_count; i++ )
            m_pan_band_list[i] = i+1;
    }
    else
    {
        for( i = 0; i < m_n_band_count; i++ )
        {
            if( ABS(m_pan_band_list[i]) > GDALGetRasterCount(m_h_dataset) )
            {
                fprintf( stderr,
                         "Band %d requested, but only bands 1 to %d available.\n",
                         ABS(m_pan_band_list[i]), GDALGetRasterCount(m_h_dataset) );
                GDALDestroyDriverManager();
                exit( 2 );
            }
        }

        if( m_n_band_count != GDALGetRasterCount( m_h_dataset ) )
            m_b_def_bands = FALSE;
    }

    /* -------------------------------------------------------------------- */
    /*      Compute the source window from the projected source window      */
    /*      if the projected coordinates were provided.  Note that the      */
    /*      projected coordinates are in ulx, uly, lrx, lry format,         */
    /*      while the anSrcWin is xoff, yoff, xsize, ysize with the         */
    /*      xoff,yoff being the ulx, uly in pixel/line.                     */
    /* -------------------------------------------------------------------- */
    if( m_df_ulx != 0.0 || m_df_uly != 0.0
            || m_df_lrx != 0.0 || m_df_lry != 0.0 )
    {
        double	adf_geo_transform[6];

        GDALGetGeoTransform( m_h_dataset, adf_geo_transform );

        if( adf_geo_transform[2] != 0.0 || adf_geo_transform[4] != 0.0 )
        {
            fprintf( stderr,
                     "The -projwin option was used, but the geotransform is\n"
                     "rotated.  This configuration is not supported.\n" );
            GDALClose( m_h_dataset );
            CPLFree( m_pan_band_list );
            GDALDestroyDriverManager();
            exit( 1 );
        }

        m_an_src_win[0] = (int)
                ((m_df_ulx - adf_geo_transform[0]) / adf_geo_transform[1] + 0.001);
        m_an_src_win[1] = (int)
                ((m_df_uly - adf_geo_transform[3]) / adf_geo_transform[5] + 0.001);

        m_an_src_win[2] = (int) ((m_df_lrx - m_df_ulx) / adf_geo_transform[1] + 0.5);
        m_an_src_win[3] = (int) ((m_df_lry - m_df_uly) / adf_geo_transform[5] + 0.5);

        if( !m_b_quiet )
            fprintf( stdout,
                     "Computed -srcwin %d %d %d %d from projected window.\n",
                     m_an_src_win[0],
                    m_an_src_win[1],
                    m_an_src_win[2],
                    m_an_src_win[3] );
    }

    /* -------------------------------------------------------------------- */
    /*      Verify source window dimensions.                                */
    /* -------------------------------------------------------------------- */
    if( m_an_src_win[2] <= 0 || m_an_src_win[3] <= 0 )
    {
        fprintf( stderr,
                 "Error: %s-srcwin %d %d %d %d has negative width and/or height.\n",
                 ( m_df_ulx != 0.0 || m_df_uly != 0.0 || m_df_lrx != 0.0 || m_df_lry != 0.0 ) ? "Computed " : "",
                 m_an_src_win[0],
                m_an_src_win[1],
                m_an_src_win[2],
                m_an_src_win[3] );
        exit( 1 );
    }

    /* -------------------------------------------------------------------- */
    /*      Verify source window dimensions.                                */
    /* -------------------------------------------------------------------- */
    else if( m_an_src_win[0] < 0 || m_an_src_win[1] < 0
             || m_an_src_win[0] + m_an_src_win[2] > GDALGetRasterXSize(m_h_dataset)
             || m_an_src_win[1] + m_an_src_win[3] > GDALGetRasterYSize(m_h_dataset) )
    {
        int b_completely_outside = m_an_src_win[0] + m_an_src_win[2] <= 0 ||
                m_an_src_win[1] + m_an_src_win[3] <= 0 ||
                m_an_src_win[0] >= GDALGetRasterXSize(m_h_dataset) ||
                m_an_src_win[1] >= GDALGetRasterYSize(m_h_dataset);
        int b_is_error = m_b_error_on_partially_outside || (b_completely_outside && m_b_error_on_completely_outside);
        if( !m_b_quiet || b_is_error )
        {
            fprintf( stderr,
                     "%s: %s-srcwin %d %d %d %d falls %s outside raster extent.%s\n",
                     (b_is_error) ? "Error" : "Warning",
                     ( m_df_ulx != 0.0 || m_df_uly != 0.0 || m_df_lrx != 0.0 || m_df_lry != 0.0 ) ? "Computed " : "",
                     m_an_src_win[0],
                    m_an_src_win[1],
                    m_an_src_win[2],
                    m_an_src_win[3],
                    (b_completely_outside) ? "completely" : "partially",
                    (b_is_error) ? "" : " Going on however." );
        }
        if( b_is_error )
            exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*      Find the output driver.                                         */
    /* -------------------------------------------------------------------- */
    m_h_driver = GDALGetDriverByName( m_psz_format );
    if( m_h_driver == NULL )
    {
        int	i_dr;

        printf( "Output driver `%s' not recognised.\n", m_psz_format );
        printf( "The following format drivers are configured and support output:\n" );
        for( i_dr = 0; i_dr < GDALGetDriverCount(); i_dr++ )
        {
            GDALDriverH hDriver = GDALGetDriver(i_dr);

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
        usage();

        GDALClose( m_h_dataset );
        CPLFree( m_pan_band_list );
        GDALDestroyDriverManager();
        CSLDestroy( argv );
        CSLDestroy( m_papsz_create_options );
        exit( 1 );
    }

    /* -------------------------------------------------------------------- */
    /*      The short form is to CreateCopy().  We use this if the input    */
    /*      matches the whole dataset.  Eventually we should rewrite        */
    /*      this entire program to use virtual datasets to construct a      */
    /*      virtual input source to copy from.                              */
    /* -------------------------------------------------------------------- */


    int b_spatial_arrangement_preserved = (
                m_an_src_win[0] == 0 && m_an_src_win[1] == 0
            && m_an_src_win[2] == GDALGetRasterXSize(m_h_dataset)
            && m_an_src_win[3] == GDALGetRasterYSize(m_h_dataset)
            && m_psz_ox_size == NULL && m_psz_oy_size == NULL );

    if( m_e_output_type == GDT_Unknown
            && !m_b_scale && !m_b_unscale
            && CSLCount(m_papsz_metadata_options) == 0 && m_b_def_bands
            && m_e_mask_mode == MASK_AUTO
            && b_spatial_arrangement_preserved
            && m_n_gcp_count == 0 && !m_b_got_bounds
            && m_psz_output_srs == NULL && !m_b_set_no_data && !m_b_unset_no_data
            && m_n_rgb_expand == 0 && !m_b_stats )
    {

        m_h_out_ds = GDALCreateCopy( m_h_driver, m_psz_dest, m_h_dataset,
                                 m_b_strict, m_papsz_create_options,
                                 m_pfn_progress, NULL );

        if( m_h_out_ds != NULL )
            GDALClose( m_h_out_ds );

        GDALClose( m_h_dataset );

        CPLFree( m_pan_band_list );

        if( !b_sub_call )
        {
            GDALDumpOpenDatasets( stderr );
            GDALDestroyDriverManager();
        }

        CSLDestroy( argv );
        CSLDestroy( m_papsz_create_options );

        return m_h_out_ds == NULL;
    }

    /* -------------------------------------------------------------------- */
    /*      Establish some parameters.                                      */
    /* -------------------------------------------------------------------- */
    if( m_psz_ox_size == NULL )
    {
        m_n_ox_size = m_an_src_win[2];
        m_n_oy_size = m_an_src_win[3];
    }
    else
    {
        m_n_ox_size = (int) ((m_psz_ox_size[strlen(m_psz_ox_size)-1]=='%'
                         ? CPLAtofM(m_psz_ox_size)/100*m_an_src_win[2] : atoi(m_psz_ox_size)));
        m_n_oy_size = (int) ((m_psz_oy_size[strlen(m_psz_oy_size)-1]=='%'
                         ? CPLAtofM(m_psz_oy_size)/100*m_an_src_win[3] : atoi(m_psz_oy_size)));
    }

    /* ==================================================================== */
    /*      Create a virtual dataset.                                       */
    /* ==================================================================== */
    VRTDataset *po_vds;

    /* -------------------------------------------------------------------- */
    /*      Make a virtual clone.                                           */
    /* -------------------------------------------------------------------- */
    po_vds = (VRTDataset *) VRTCreate( m_n_ox_size, m_n_oy_size );

    if( m_n_gcp_count == 0 )
    {
        if( m_psz_output_srs != NULL )
        {
            po_vds->SetProjection( m_psz_output_srs );
        }
        else
        {
            m_psz_projection = GDALGetProjectionRef( m_h_dataset );
            if( m_psz_projection != NULL && strlen(m_psz_projection) > 0 )
                po_vds->SetProjection( m_psz_projection );
        }
    }

    if( m_b_got_bounds )
    {
        m_adf_geo_transform[0] = m_adf_ullr[0];
        m_adf_geo_transform[1] = (m_adf_ullr[2] - m_adf_ullr[0]) / m_n_ox_size;
        m_adf_geo_transform[2] = 0.0;
        m_adf_geo_transform[3] = m_adf_ullr[1];
        m_adf_geo_transform[4] = 0.0;
        m_adf_geo_transform[5] = (m_adf_ullr[3] - m_adf_ullr[1]) / m_n_oy_size;

        po_vds->SetGeoTransform( m_adf_geo_transform );
    }

    else if( GDALGetGeoTransform( m_h_dataset, m_adf_geo_transform ) == CE_None
             && m_n_gcp_count == 0 )
    {
        m_adf_geo_transform[0] += m_an_src_win[0] * m_adf_geo_transform[1]
                + m_an_src_win[1] * m_adf_geo_transform[2];
        m_adf_geo_transform[3] += m_an_src_win[0] * m_adf_geo_transform[4]
                + m_an_src_win[1] * m_adf_geo_transform[5];

        m_adf_geo_transform[1] *= m_an_src_win[2] / (double) m_n_ox_size;
        m_adf_geo_transform[2] *= m_an_src_win[3] / (double) m_n_oy_size;
        m_adf_geo_transform[4] *= m_an_src_win[2] / (double) m_n_ox_size;
        m_adf_geo_transform[5] *= m_an_src_win[3] / (double) m_n_oy_size;

        po_vds->SetGeoTransform( m_adf_geo_transform );
    }

    if( m_n_gcp_count != 0 )
    {
        const char *pszGCPProjection = m_psz_output_srs;

        if( pszGCPProjection == NULL )
            pszGCPProjection = GDALGetGCPProjection( m_h_dataset );
        if( pszGCPProjection == NULL )
            pszGCPProjection = "";

        po_vds->SetGCPs( m_n_gcp_count, m_pas_gcps, pszGCPProjection );

        GDALDeinitGCPs( m_n_gcp_count, m_pas_gcps );
        CPLFree( m_pas_gcps );
    }

    else if( GDALGetGCPCount( m_h_dataset ) > 0 )
    {
        GDAL_GCP *pas_gcps;
        int       n_gcps = GDALGetGCPCount( m_h_dataset );

        pas_gcps = GDALDuplicateGCPs( n_gcps, GDALGetGCPs( m_h_dataset ) );

        for( i = 0; i < n_gcps; i++ )
        {
            pas_gcps[i].dfGCPPixel -= m_an_src_win[0];
            pas_gcps[i].dfGCPLine  -= m_an_src_win[1];
            pas_gcps[i].dfGCPPixel *= (m_n_ox_size / (double) m_an_src_win[2] );
            pas_gcps[i].dfGCPLine  *= (m_n_oy_size / (double) m_an_src_win[3] );
        }

        po_vds->SetGCPs( n_gcps, pas_gcps,
                        GDALGetGCPProjection( m_h_dataset ) );

        GDALDeinitGCPs( n_gcps, pas_gcps );
        CPLFree( pas_gcps );
    }

    /* -------------------------------------------------------------------- */
    /*      To make the VRT to look less awkward (but this is optional      */
    /*      in fact), avoid negative values.                                */
    /* -------------------------------------------------------------------- */
    int an_dst_win[4];
    an_dst_win[0] = 0;
    an_dst_win[1] = 0;
    an_dst_win[2] = m_n_ox_size;
    an_dst_win[3] = m_n_oy_size;

    fixSrcDstWindow( m_an_src_win, an_dst_win,
                     GDALGetRasterXSize(m_h_dataset),
                     GDALGetRasterYSize(m_h_dataset) );

    /* -------------------------------------------------------------------- */
    /*      Transfer generally applicable metadata.                         */
    /* -------------------------------------------------------------------- */
    char** papsz_metadata = CSLDuplicate(((GDALDataset*)m_h_dataset)->GetMetadata());
    if ( m_b_scale || m_b_unscale || m_e_output_type != GDT_Unknown )
    {
        /* Remove TIFFTAG_MINSAMPLEVALUE and TIFFTAG_MAXSAMPLEVALUE */
        /* if the data range may change because of options */
        char** papsz_iter = papsz_metadata;
        while(papsz_iter && *papsz_iter)
        {
            if (EQUALN(*papsz_iter, "TIFFTAG_MINSAMPLEVALUE=", 23) ||
                    EQUALN(*papsz_iter, "TIFFTAG_MAXSAMPLEVALUE=", 23))
            {
                CPLFree(*papsz_iter);
                memmove(papsz_iter, papsz_iter+1, sizeof(char*) * (CSLCount(papsz_iter+1)+1));
            }
            else
                papsz_iter++;
        }
    }
    po_vds->SetMetadata( papsz_metadata );
    CSLDestroy( papsz_metadata );
    attachMetadata( (GDALDatasetH) po_vds, m_papsz_metadata_options );

    const char* psz_interleave = GDALGetMetadataItem(m_h_dataset, "INTERLEAVE", "IMAGE_STRUCTURE");
    if (psz_interleave)
        po_vds->SetMetadataItem("INTERLEAVE", psz_interleave, "IMAGE_STRUCTURE");

    /* -------------------------------------------------------------------- */
    /*      Transfer metadata that remains valid if the spatial             */
    /*      arrangement of the data is unaltered.                           */
    /* -------------------------------------------------------------------- */
    if( b_spatial_arrangement_preserved )
    {
        char **papsz_md;

        papsz_md = ((GDALDataset*)m_h_dataset)->GetMetadata("RPC");
        if( papsz_md != NULL )
            po_vds->SetMetadata( papsz_md, "RPC" );

        papsz_md = ((GDALDataset*)m_h_dataset)->GetMetadata("GEOLOCATION");
        if( papsz_md != NULL )
            po_vds->SetMetadata( papsz_md, "GEOLOCATION" );
    }

    int n_src_band_count = m_n_band_count;

    if (m_n_rgb_expand != 0)
    {
        GDALRasterBand  *po_src_band;
        po_src_band = ((GDALDataset *)
                     m_h_dataset)->GetRasterBand(ABS(m_pan_band_list[0]));
        if (m_pan_band_list[0] < 0)
            po_src_band = po_src_band->GetMaskBand();
        GDALColorTable* po_color_table = po_src_band->GetColorTable();
        if (po_color_table == NULL)
        {
            fprintf(stderr, "Error : band %d has no color table\n", ABS(m_pan_band_list[0]));
            GDALClose( m_h_dataset );
            CPLFree( m_pan_band_list );
            GDALDestroyDriverManager();
            CSLDestroy( argv );
            CSLDestroy( m_papsz_create_options );
            exit( 1 );
        }

        /* Check that the color table only contains gray levels */
        /* when using -expand gray */
        if (m_n_rgb_expand == 1)
        {
            int n_color_count = po_color_table->GetColorEntryCount();
            int n_color;
            for( n_color = 0; n_color < n_color_count; n_color++ )
            {
                const GDALColorEntry* poEntry = po_color_table->GetColorEntry(n_color);
                if (poEntry->c1 != poEntry->c2 || poEntry->c1 != poEntry->c2)
                {
                    fprintf(stderr, "Warning : color table contains non gray levels colors\n");
                    break;
                }
            }
        }

        if (m_n_band_count == 1)
            m_n_band_count = m_n_rgb_expand;
        else if (m_n_band_count == 2 && (m_n_rgb_expand == 3 || m_n_rgb_expand == 4))
            m_n_band_count = m_n_rgb_expand;
        else
        {
            fprintf(stderr, "Error : invalid use of -expand option.\n");
            exit( 1 );
        }
    }

    int bFilterOutStatsMetadata =
            (m_b_scale || m_b_unscale || !b_spatial_arrangement_preserved || m_n_rgb_expand != 0);

    /* ==================================================================== */
    /*      Process all bands.                                              */
    /* ==================================================================== */
    for( i = 0; i < m_n_band_count; i++ )
    {
        VRTSourcedRasterBand   *po_vrt_band;
        GDALRasterBand  *po_src_band;
        GDALDataType    e_band_type;
        int             n_component = 0;

        int n_src_band;
        if (m_n_rgb_expand != 0)
        {
            if (n_src_band_count == 2 && m_n_rgb_expand == 4 && i == 3)
                n_src_band = m_pan_band_list[1];
            else
            {
                n_src_band = m_pan_band_list[0];
                n_component = i + 1;
            }
        }
        else
            n_src_band = m_pan_band_list[i];

        po_src_band = ((GDALDataset *) m_h_dataset)->GetRasterBand(ABS(n_src_band));

        /* -------------------------------------------------------------------- */
        /*      Select output data type to match source.                        */
        /* -------------------------------------------------------------------- */
        if( m_e_output_type == GDT_Unknown )
            e_band_type = po_src_band->GetRasterDataType();
        else
            e_band_type = m_e_output_type;

        /* -------------------------------------------------------------------- */
        /*      Create this band.                                               */
        /* -------------------------------------------------------------------- */
        po_vds->AddBand( e_band_type, NULL );
        po_vrt_band = (VRTSourcedRasterBand *) po_vds->GetRasterBand( i+1 );
        if (n_src_band < 0)
        {
            po_vrt_band->AddMaskBandSource(po_src_band,
                                         m_an_src_win[0], m_an_src_win[1],
                    m_an_src_win[2], m_an_src_win[3],
                    an_dst_win[0], an_dst_win[1],
                    an_dst_win[2], an_dst_win[3]);
            continue;
        }

        /* -------------------------------------------------------------------- */
        /*      Do we need to collect scaling information?                      */
        /* -------------------------------------------------------------------- */
        double df_scale=1.0, df_offset=0.0;

        if( m_b_scale && !m_b_have_scale_src )
        {
            double	adf_c_min_max[2];
            GDALComputeRasterMinMax( po_src_band, TRUE, adf_c_min_max );
            m_df_scale_src_min = adf_c_min_max[0];
            m_df_scale_src_max = adf_c_min_max[1];
        }

        if( m_b_scale )
        {
            if( m_df_scale_src_max == m_df_scale_src_min )
                m_df_scale_src_max += 0.1;
            if( m_df_scale_dst_max == m_df_scale_dst_min )
                m_df_scale_dst_max += 0.1;

            df_scale = (m_df_scale_dst_max - m_df_scale_dst_min)
                    / (m_df_scale_src_max - m_df_scale_src_min);
            df_offset = -1 * m_df_scale_src_min * df_scale + m_df_scale_dst_min;
        }

        if( m_b_unscale )
        {
            df_scale = po_src_band->GetScale();
            df_offset = po_src_band->GetOffset();
        }

        /* -------------------------------------------------------------------- */
        /*      Create a simple or complex data source depending on the         */
        /*      translation type required.                                      */
        /* -------------------------------------------------------------------- */
        if( m_b_unscale || m_b_scale || (m_n_rgb_expand != 0 && i < m_n_rgb_expand) )
        {
            po_vrt_band->AddComplexSource( po_src_band,
                                         m_an_src_win[0], m_an_src_win[1],
                    m_an_src_win[2], m_an_src_win[3],
                    an_dst_win[0], an_dst_win[1],
                    an_dst_win[2], an_dst_win[3],
                    df_offset, df_scale,
                    VRT_NODATA_UNSET,
                    n_component );
        }
        else
            po_vrt_band->AddSimpleSource( po_src_band,
                                        m_an_src_win[0], m_an_src_win[1],
                    m_an_src_win[2], m_an_src_win[3],
                    an_dst_win[0], an_dst_win[1],
                    an_dst_win[2], an_dst_win[3] );

        /* -------------------------------------------------------------------- */
        /*      In case of color table translate, we only set the color         */
        /*      interpretation other info copied by CopyBandInfo are            */
        /*      not relevant in RGB expansion.                                  */
        /* -------------------------------------------------------------------- */
        if (m_n_rgb_expand == 1)
        {
            po_vrt_band->SetColorInterpretation( GCI_GrayIndex );
        }
        else if (m_n_rgb_expand != 0 && i < m_n_rgb_expand)
        {
            po_vrt_band->SetColorInterpretation( (GDALColorInterp) (GCI_RedBand + i) );
        }

        /* -------------------------------------------------------------------- */
        /*      copy over some other information of interest.                   */
        /* -------------------------------------------------------------------- */
        else
        {
            copyBandInfo( po_src_band, po_vrt_band,
                          !m_b_stats && !bFilterOutStatsMetadata,
                          !m_b_unscale,
                          !m_b_set_no_data && !m_b_unset_no_data );
        }

        /* -------------------------------------------------------------------- */
        /*      Set a forcable nodata value?                                    */
        /* -------------------------------------------------------------------- */
        if( m_b_set_no_data )
        {
            double df_val = m_df_no_data_real;
            int b_clamped = FALSE, b_rounded = FALSE;

#define CLAMP(val,type,minval,maxval) \
    do { if (val < minval) { b_clamped = TRUE; val = minval; } \
    else if (val > maxval) { b_clamped = TRUE; val = maxval; } \
    else if (val != (type)val) { b_rounded = TRUE; val = (type)(val + 0.5); } } \
    while(0)

            switch(e_band_type)
            {
            case GDT_Byte:
                CLAMP(df_val, GByte, 0.0, 255.0);
                break;
            case GDT_Int16:
                CLAMP(df_val, GInt16, -32768.0, 32767.0);
                break;
            case GDT_UInt16:
                CLAMP(df_val, GUInt16, 0.0, 65535.0);
                break;
            case GDT_Int32:
                CLAMP(df_val, GInt32, -2147483648.0, 2147483647.0);
                break;
            case GDT_UInt32:
                CLAMP(df_val, GUInt32, 0.0, 4294967295.0);
                break;
            default:
                break;
            }

            if (b_clamped)
            {
                printf( "for band %d, nodata value has been clamped "
                        "to %.0f, the original value being out of range.\n",
                        i + 1, df_val);
            }
            else if(b_rounded)
            {
                printf("for band %d, nodata value has been rounded "
                       "to %.0f, %s being an integer datatype.\n",
                       i + 1, df_val,
                       GDALGetDataTypeName(e_band_type));
            }

            po_vrt_band->SetNoDataValue( df_val );
        }

        if (m_e_mask_mode == MASK_AUTO &&
                (GDALGetMaskFlags(GDALGetRasterBand(m_h_dataset, 1)) & GMF_PER_DATASET) == 0 &&
                (po_src_band->GetMaskFlags() & (GMF_ALL_VALID | GMF_NODATA)) == 0)
        {
            if (po_vrt_band->CreateMaskBand(po_src_band->GetMaskFlags()) == CE_None)
            {
                VRTSourcedRasterBand* hMaskVRTBand =
                        (VRTSourcedRasterBand*)po_vrt_band->GetMaskBand();
                hMaskVRTBand->AddMaskBandSource(po_src_band,
                                                m_an_src_win[0], m_an_src_win[1],
                        m_an_src_win[2], m_an_src_win[3],
                        an_dst_win[0], an_dst_win[1],
                        an_dst_win[2], an_dst_win[3] );
            }
        }
    }

    if (m_e_mask_mode == MASK_USER)
    {
        GDALRasterBand *poSrcBand =
                (GDALRasterBand*)GDALGetRasterBand(m_h_dataset, ABS(m_n_mask_band));
        if (poSrcBand && po_vds->CreateMaskBand(GMF_PER_DATASET) == CE_None)
        {
            VRTSourcedRasterBand* hMaskVRTBand = (VRTSourcedRasterBand*)
                    GDALGetMaskBand(GDALGetRasterBand((GDALDatasetH)po_vds, 1));
            if (m_n_mask_band > 0)
                hMaskVRTBand->AddSimpleSource(poSrcBand,
                                              m_an_src_win[0], m_an_src_win[1],
                        m_an_src_win[2], m_an_src_win[3],
                        an_dst_win[0], an_dst_win[1],
                        an_dst_win[2], an_dst_win[3] );
            else
                hMaskVRTBand->AddMaskBandSource(poSrcBand,
                                                m_an_src_win[0], m_an_src_win[1],
                        m_an_src_win[2], m_an_src_win[3],
                        an_dst_win[0], an_dst_win[1],
                        an_dst_win[2], an_dst_win[3] );
        }
    }
    else
        if (m_e_mask_mode == MASK_AUTO && n_src_band_count > 0 &&
                GDALGetMaskFlags(GDALGetRasterBand(m_h_dataset, 1)) == GMF_PER_DATASET)
        {
            if (po_vds->CreateMaskBand(GMF_PER_DATASET) == CE_None)
            {
                VRTSourcedRasterBand* hMaskVRTBand = (VRTSourcedRasterBand*)
                        GDALGetMaskBand(GDALGetRasterBand((GDALDatasetH)po_vds, 1));
                hMaskVRTBand->AddMaskBandSource((GDALRasterBand*)GDALGetRasterBand(m_h_dataset, 1),
                                                m_an_src_win[0], m_an_src_win[1],
                        m_an_src_win[2], m_an_src_win[3],
                        an_dst_win[0], an_dst_win[1],
                        an_dst_win[2], an_dst_win[3] );
            }
        }

    /* -------------------------------------------------------------------- */
    /*      Compute stats if required.                                      */
    /* -------------------------------------------------------------------- */
    if (m_b_stats)
    {
        for( i = 0; i < po_vds->GetRasterCount(); i++ )
        {
            double df_min, df_max, df_mean, df_std_dev;
            po_vds->GetRasterBand(i+1)->ComputeStatistics( m_b_approx_stats,
                                                          &df_min, &df_max, &df_mean, &df_std_dev, GDALDummyProgress, NULL );
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Write to the output file using CopyCreate().                    */
    /* -------------------------------------------------------------------- */
    m_h_out_ds = GDALCreateCopy( m_h_driver, m_psz_dest, (GDALDatasetH) po_vds,
                             m_b_strict, m_papsz_create_options,
                             m_pfn_progress, NULL );
    if( m_h_out_ds != NULL )
    {
        int bHasGotErr = FALSE;
        CPLErrorReset();
        GDALFlushCache( m_h_out_ds );
        if (CPLGetLastErrorType() != CE_None)
            bHasGotErr = TRUE;
        GDALClose( m_h_out_ds );
        if (bHasGotErr)
            m_h_out_ds = NULL;
    }

    GDALClose( (GDALDatasetH) po_vds );

    GDALClose( m_h_dataset );

    CPLFree( m_pan_band_list );

    CPLFree( m_psz_output_srs );

    if( !b_sub_call )
    {
        GDALDumpOpenDatasets( stderr );
        GDALDestroyDriverManager();
    }

    CSLDestroy( argv );
    CSLDestroy( m_papsz_create_options );

    return m_h_out_ds == NULL;
}

} // namespace nav_tools

