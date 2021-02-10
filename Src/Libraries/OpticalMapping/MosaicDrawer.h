#ifndef MOSAICDRAWER_H
#define MOSAICDRAWER_H


#include "MosaicDescriptor.h"


struct drawingOptions{
    bool tryGpu;
    double seamMegapix;
    int exposCompType;
    bool gainBlock;
    QString seamFindType;
    int blendType;
    float blendStrength;
};

class MosaicDrawer
{
public:

    ///
    /// \brief MosaicDrawer		This class is used to blend and draw a Mosaic
    ///
    ///
    /// \param drawingOptions : this parameter gives options for the drawing
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
    MosaicDrawer(QString drawingOptions =
            QString("--try_gpu no --seam gc_color --expos_comp gain --blend multiband"));

    ///
    /// \brief drawAndBlend draw and blend mosaic in a single memory image using mosaic descriptor
    /// \param mosaicD_p MosaicDescriptor for the mosaic to be blended
    /// \param mosaicImage_p Blended mosaic image
    /// \param mosaicImageMask_p Blended mosaic mask (0 for non mosaic areas)
    ///
    void drawAndBlend(const MosaicDescriptor & mosaicD_p, cv::UMat & mosaicImage_p , cv::UMat & mosaicImageMask_p);

    ///
    /// \brief drawAndBlend draw & blend mosaic in a single memory image using list of images, masks and corners_p
    /// \param imagesWarped_p images that have been transformed (ready to mosaic and only need a translation)
    /// \param masksWarped_p masks corresponding to imagesWarped_p (0 for non valid areas)
    /// \param corners_p top left corner of each image (for images translation)
    /// \param mosaicImage_p Blended mosaic image
    /// \param mosaicImageMask_p Blended mosaic mask (0 for non mosaic areas)
    ///
    void drawAndBlend(std::vector<cv::UMat> & imagesWarped_p,
                      std::vector<cv::UMat> & masksWarped_p,
                      std::vector<cv::Point> & corners_p,
                      cv::UMat & mosaicImage_p,
                      cv::UMat & mosaicImageMask_p);

    ///
    /// \brief blockDrawBlendAndWrite This function draw a mosaic by block in order to optimize the memory used for drawing
    /// \param mosaicD_p MosaicDescriptor for the mosaic to be blended
    /// \param blockSize_p x and y size in pixel of a single block
    /// \param writingFolderPath_p path to write mosic files
    /// \return list of output filenames
    ///
    QStringList blockDrawBlendAndWrite(const MosaicDescriptor &mosaicD_p,
                                cv::Point2d blockSize_p,
                                QString writingPath_p,
                                QString prefix_p);

    ///
    /// \brief blockDrawBlendAndWrite This function draw each image as a single geotiff file
    /// \param mosaicD_p MosaicDescriptor for the mosaic to be blended
    /// \param writingFolderPath_p path to write mosic files
    /// \return list of output filenames
    ///
    QStringList writeImagesAsGeoTiff(const MosaicDescriptor& mosaicD_p,
        QString writingPath_p,
        QString prefix_p);

    int parseAndAffectOptions(QString drawingOptions);

private:

    drawingOptions dOptions;




};

#endif // MOSAICDRAWER_H
