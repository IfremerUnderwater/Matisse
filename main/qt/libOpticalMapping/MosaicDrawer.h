#ifndef MOSAICDRAWER_H
#define MOSAICDRAWER_H

#include "MosaicDescriptor.h"

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

    void drawAndBlend(const MosaicDescriptor & mosaicD_p, cv::Mat & mosaicImage_p , cv::Mat & mosaicImageMask_p);

    void drawAndBlend(std::vector<Mat> & images_warped,
                      std::vector<Mat> & masks_warped,
                      std::vector<Point> & corners,
                      cv::Mat & mosaicImage_p,
                      cv::Mat & mosaicImageMask_p);

    int parseAndAffectOptions(QString drawingOptions);

private:

    bool _tryGpu;
    double _seamMegapix;
    int _exposCompType;
    bool _gainBlock;
    QString _seamFindType;
    int _blendType;
    float _blendStrength;


};

#endif // MOSAICDRAWER_H
