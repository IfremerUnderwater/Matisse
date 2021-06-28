#ifndef NAV_TOOLS_RASTER_GEO_REFERENCER_H_
#define NAV_TOOLS_RASTER_GEO_REFERENCER_H_



#ifdef  _WIN32
#include <cpl_vsi.h>
#include <cpl_conv.h>
#include <cpl_string.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <vrtdataset.h>
#else
#include <gdal/cpl_vsi.h>
#include <gdal/cpl_conv.h>
#include <gdal/cpl_string.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogr_spatialref.h>
#include <gdal/vrtdataset.h>
#endif
#include <opencv2/opencv.hpp>
#include <QString>
#include <QStringList>

namespace nav_tools {

/*! This class is an encapsulation of gdal_translate code to create GeoTiff from a memory raster*/

class RasterGeoreferencer
{
public:
    RasterGeoreferencer();
    ~RasterGeoreferencer();

    ///
    /// \brief writeGeoFile write a georeferenced file using memory raster (this method is the class version of the command line gdal_translate)
    /// \param _raster opencv image of the raster data
    /// \param _raster_mask opencv image of the raster mask (0 not to be seen, 255 valid area)
    /// \param _output_file path and name of the output geofile
    /// \param _cmd_line_options gdal options as would be provided to gdal_translate function (see gdal doc for exhaustive list)
    /// \return 0 on success
    ///
    int writeGeoFile(cv::Mat &_raster, cv::Mat &_raster_mask, QString _output_file, QString _cmd_line_options);

private:

    GDALDataset         *m_po_dataset;
    GDALDatasetH	    m_h_dataset, m_h_out_ds;
    int			        m_n_raster_x_size, m_n_raster_y_size;
    const char		    *m_psz_dest, *m_psz_format ;
    int                 m_b_format_explicitely_set;
    GDALDriverH		    m_h_driver;
    GDALDriver          *m_input_driver;
    int			        *m_pan_band_list; /* negative value of panBandList[i] means mask band of ABS(panBandList[i]) */
    int                 m_n_band_count, m_b_def_bands;
    double		        *m_adf_geo_transform;
    GDALDataType	    m_e_output_type;
    int			        m_n_ox_size, m_n_oy_size;
    char		        *m_psz_ox_size, *m_psz_oy_size;
    char                **m_papsz_create_options;
    int                 *m_an_src_win, m_b_strict;
    const char          *m_psz_projection;
    int                 m_b_scale, m_b_have_scale_src, m_b_unscale;
    double	            m_df_scale_src_min, m_df_scale_src_max;
    double              m_df_scale_dst_min, m_df_scale_dst_max;
    double              m_df_ulx, m_df_uly, m_df_lrx, m_df_lry;
    char                **m_papsz_metadata_options;
    char                *m_psz_output_srs;
    int                 m_b_quiet, m_b_got_bounds;
    GDALProgressFunc    m_pfn_progress;
    int                 m_n_gcp_count;
    GDAL_GCP            *m_pas_gcps;
    int                 m_i_src_file_arg, m_i_dst_file_arg;
    int                 b_copy_sub_datasets;
    double              *m_adf_ullr;
    int                 m_b_set_no_data;
    int                 m_b_unset_no_data;
    double		        m_df_no_data_real;
    int                 m_n_rgb_expand;
    int                 m_b_parsed_mask_argument;
    int                 m_e_mask_mode;
    int                 m_n_mask_band;
    int                 m_b_stats, m_b_approx_stats;
    int                 m_b_error_on_partially_outside;
    int                 m_b_error_on_completely_outside;

};

}

#endif // NAV_TOOLS_RASTER_GEO_REFERENCER_H_
