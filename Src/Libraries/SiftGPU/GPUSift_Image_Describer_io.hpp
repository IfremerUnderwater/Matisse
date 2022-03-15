#ifndef OPENMVG_FEATURES_SIFT_GPUSIFT_IMAGE_DESCRIBER_IO_HPP
#define OPENMVG_FEATURES_SIFT_GPUSIFT_IMAGE_DESCRIBER_IO_HPP

#include "GpuSift_Image_Describer.hpp"

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>

template<class Archive>
inline void openMVG::features::GpuSift_Image_describer::Params::serialize( Archive & ar )
{
  ar(
    cereal::make_nvp("first_octave", first_octave_),
    cereal::make_nvp("num_octaves",num_octaves_),
    cereal::make_nvp("num_scales",num_scales_),
    cereal::make_nvp("edge_threshold",edge_threshold_),
    cereal::make_nvp("peak_threshold",peak_threshold_),
    cereal::make_nvp("root_sift",root_sift_));
}


template<class Archive>
inline void openMVG::features::GpuSift_Image_describer::serialize( Archive & ar )
{
  ar(cereal::make_nvp("params", params_));
}


CEREAL_REGISTER_TYPE_WITH_NAME(openMVG::features::GpuSift_Image_describer, "GpuSift_Image_describer");
CEREAL_REGISTER_POLYMORPHIC_RELATION(openMVG::features::Image_describer, openMVG::features::GpuSift_Image_describer)

#endif // OPENMVG_FEATURES_SIFT_GPUSIFT_IMAGE_DESCRIBER_IO_HPP