#ifndef OPTICAL_MAPPING_MOSAIC_DRAWER_H_
#define OPTICAL_MAPPING_MOSAIC_DRAWER_H_


#include "mosaic_descriptor.h"

namespace optical_mapping {

struct drawingOptions{
    bool try_gpu;
    double seam_megapix;
    int expos_comp_type;
    bool gain_block;
    QString seam_find_type;
    int blend_type;
    float blend_strength;
};

class MosaicDrawer
{
public:

    ///
    /// \brief MosaicDrawer		This class is used to blend and draw a Mosaic
    ///
    ///
    /// \param _drawing_options : this parameter gives options for the drawing
    ///        in an equivalent way as argc, argv
    ///        with the following possible options
    ///
    ///   	  --try_gpu (yes|no)
    ///		      Try to use GPU. The default value is 'no'. All default values
    ///		      are for CPU mode.
    ///
    ///          Compositing Flags:
    ///		  --seam_megapix <float>
    ///		      Resolution for seam estimation step. The default is 0.1 Mpx.
    ///		  --seam (no|voronoi|gc_color|gc_colorgrad)
    ///		      Seam estimation method. The default is 'gc_color'.
    ///		  --expos_comp (no|gain|gain_blocks)
    ///		      Exposure compensation method. The default is 'gain_blocks'.
    ///		  --blend (no|feather|multiband)
    ///		      Blending method. The default is 'multiband'.
    ///		  --blend_strength <float>
    ///		      Blending strength from [0,100] range. The default is 5.
    ///
    MosaicDrawer(QString _drawing_Options =
            QString("--try_gpu no --seam gc_color --expos_comp gain --blend multiband"));

    ///
    /// \brief drawAndBlend draw and blend mosaic in a single memory image using mosaic descriptor
    /// \param _mosaic_d_p MosaicDescriptor for the mosaic to be blended
    /// \param _mosaic_image_p Blended mosaic image
    /// \param _mosaic_image_mask_p Blended mosaic mask (0 for non mosaic areas)
    ///
    void drawAndBlend(const MosaicDescriptor & _mosaic_d_p, cv::UMat & _mosaic_image_p , cv::UMat & _mosaic_image_mask_p);

    ///
    /// \brief drawAndBlend draw & blend mosaic in a single memory image using list of images, masks and corners_p
    /// \param _images_warped_p images that have been transformed (ready to mosaic and only need a translation)
    /// \param _masks_warped_p masks corresponding to imagesWarped_p (0 for non valid areas)
    /// \param _corners_p top left corner of each image (for images translation)
    /// \param _mosaic_image_p Blended mosaic image
    /// \param _mosaic_image_mask_p Blended mosaic mask (0 for non mosaic areas)
    ///
    void drawAndBlend(std::vector<cv::UMat> & _images_warped_p,
                      std::vector<cv::UMat> & _masks_warped_p,
                      std::vector<cv::Point> & _corners_p,
                      cv::UMat & _mosaic_image_p,
                      cv::UMat & _mosaic_image_mask_p);

    ///
    /// \brief blockDrawBlendAndWrite This function draw a mosaic by block in order to optimize the memory used for drawing
    /// \param _mosaic_d_p MosaicDescriptor for the mosaic to be blended
    /// \param _block_size_p x and y size in pixel of a single block
    /// \param _writing_path_p path to write mosic files
    /// \param _prefix_p file name prefix
    /// \return list of output filenames
    ///
    QStringList blockDrawBlendAndWrite(const MosaicDescriptor &_mosaic_d_p,
                                cv::Point2d _block_size_p,
                                QString _writing_path_p,
                                QString _prefix_p);

    ///
    /// \brief writeImagesAsGeoTiff This function draw each image as a single geotiff file
    /// \param _mosaic_d_p MosaicDescriptor for the mosaic to be drawn
    /// \param _writing_path_p path to write mosic files
    /// \param _prefix_p file name prefix
    /// \return list of output filenames
    ///
    QStringList writeImagesAsGeoTiff(const MosaicDescriptor& _mosaic_d_p,
        QString _writing_path_p,
        QString _prefix_p);

    ///
/// \brief blockDrawBlendAndWrite This function output each image without transform
/// \param _mosaic_d_p MosaicDescriptor for the mosaic
/// \param _writing_path_p path to write mosic files
/// \param _prefix_p file name prefix
/// \return list of output filenames
///
    QStringList outputMosaicImagesAsIs(const MosaicDescriptor& _mosaic_d_p,
        QString _writing_path_p,
        QString _prefix_p);

    int parseAndAffectOptions(QString _drawing_options);

private:

    drawingOptions m_d_options;




};

} // namespace optical_mapping

#endif // OPTICAL_MAPPING_MOSAIC_DRAWER_H_
