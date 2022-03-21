#ifndef OPENMVG_FEATURES_SIFT_GPUSIFT_IMAGE_DESCRIBER_HPP
#define OPENMVG_FEATURES_SIFT_GPUSIFT_IMAGE_DESCRIBER_HPP

#include <numeric>
#include <vector>

#include "openMVG/features/feature.hpp"
#include "openMVG/features/image_describer.hpp"
#include "openMVG/features/regions_factory.hpp"
#include "openMVG/features/sift/hierarchical_gaussian_scale_space.hpp"
#include "openMVG/features/sift/sift_DescriptorExtractor.hpp"
#include "openMVG/features/sift/sift_keypoint.hpp"
#include "openMVG/features/sift/sift_KeypointExtractor.hpp"
#include "openMVG/system/logger.hpp"
#include "SiftGPU.h"
#include "GL/glew.h"


namespace openMVG {
namespace features {

class GpuSift_Image_describer : public Image_describer
{
public:

  using Regions_type = SIFT_Regions;

  struct Params
  {
    Params(
      int first_octave = 0,
      int num_octaves = 6,
      int num_scales = 3,
      float edge_threshold = 10.0f,
      float peak_threshold = 0.04f,
      bool root_sift = true
    ):
      first_octave_(first_octave),
      num_octaves_(num_octaves),
      num_scales_(num_scales),
      edge_threshold_(edge_threshold),
      peak_threshold_(peak_threshold),
      root_sift_(root_sift) {}

    template<class Archive>
    inline void serialize( Archive & ar );

    // Parameters
    int first_octave_;      // Use original image, or perform an upscale if == -1
    int num_octaves_;       // Max octaves count
    int num_scales_;        // Scales per octave
    float edge_threshold_;  // Max ratio of Hessian eigenvalues
    float peak_threshold_;  // Min contrast
    bool root_sift_;        // see [1]
  };

  explicit GpuSift_Image_describer(const Params & params = Params(), SiftGPU* sift_gpu=nullptr):
    Image_describer(),
    params_(params),
      sift_gpu_(sift_gpu)
  {
      if (!sift_gpu)
      {
          sift_gpu_ = new SiftGPU;
          //if (sift_gpu_->CreateContextGL() != SiftGPU::SIFTGPU_FULL_SUPPORTED)

          //if (sift_gpu_->VerifyContextGL() != SiftGPU::SIFTGPU_FULL_SUPPORTED)
          //{
          //    delete sift_gpu_;
          //    sift_gpu_ = nullptr;
          //}
      }

      if (sift_gpu_)
      {
          UpdateConfigFromParam();
      }

  }

  void UpdateConfigFromParam()
  {
      std::vector<std::string> sift_gpu_args;

      sift_gpu_args.push_back("./sift_gpu");

      sift_gpu_args.push_back("-da");
      // No verbose logging.
      sift_gpu_args.push_back("-v");
      sift_gpu_args.push_back("1");

      // Fixed maximum image dimension.
      sift_gpu_args.push_back("-maxd");
      sift_gpu_args.push_back(std::to_string(32000));

      // Keep the highest level features.
      //sift_gpu_args.push_back("-tc2");
      //sift_gpu_args.push_back(std::to_string(params_.max_num_features));
      //sift_gpu_args.push_back(std::to_string(8000));

      // First octave level.
      sift_gpu_args.push_back("-fo");
      sift_gpu_args.push_back(std::to_string(params_.first_octave_));

      // Number of dog levels in an octave.
      sift_gpu_args.push_back("-d");
      sift_gpu_args.push_back(std::to_string(params_.num_scales_));

      // Number of octave levels.
      //sift_gpu_args.push_back("-d");
      //sift_gpu_args.push_back(std::to_string(params_.num_scales_));

      // Peak threshold.
      sift_gpu_args.push_back("-t");
      sift_gpu_args.push_back(std::to_string(params_.peak_threshold_));

      // Edge threshold.
      sift_gpu_args.push_back("-e");
      sift_gpu_args.push_back(std::to_string(params_.edge_threshold_));

      std::vector<const char*> sift_gpu_args_cstr;
      sift_gpu_args_cstr.reserve(sift_gpu_args.size());
      for (const auto& arg : sift_gpu_args) {
          sift_gpu_args_cstr.push_back(arg.c_str());
      }

      sift_gpu_->ParseParam(sift_gpu_args_cstr.size(), sift_gpu_args_cstr.data());
  }

  bool Set_configuration_preset(EDESCRIBER_PRESET preset) override
  {
    switch (preset)
    {
    case NORMAL_PRESET:
      params_.peak_threshold_ = 0.04f/3;
    break;
    case HIGH_PRESET:
      params_.peak_threshold_ = 0.02f/3;
    break;
    case ULTRA_PRESET:
      params_.peak_threshold_ = 0.02f/3;
      params_.first_octave_ = -1;
    break;
    default:
      OPENMVG_LOG_ERROR << "Invalid preset configuration";
      return false;
    }
    UpdateConfigFromParam();

    if (sift_gpu_->VerifyContextGL() != SiftGPU::SIFTGPU_FULL_SUPPORTED)
        return false;

    return true;
  }

  /**
  @brief Detect regions on the image and compute their attributes (description)
  @param image Image.
  @param mask 8-bit gray image for keypoint filtering (optional).
     Non-zero values depict the region of interest.
  @return regions The detected regions and attributes (the caller must delete the allocated data)
  */
  std::unique_ptr<Regions_type> Describe_GpuSift(
    const image::Image<unsigned char>& image,
    const image::Image<unsigned char>* mask = nullptr
  )
  {
    auto regions = std::unique_ptr<Regions_type>(new Regions_type);

    //UpdateConfigFromParam();

    if (image.size() == 0)
      return regions;

    std::vector<float > descriptors(1);
    std::vector<SiftGPU::SiftKeypoint> keys(1);
    int num = 0;

    // compute sift
    if (sift_gpu_->RunSIFT(image.GetMat().cols(), image.GetMat().rows(), image.GetMat().data(), GL_LUMINANCE, GL_UNSIGNED_BYTE))
    {

        //get feature count
        num = sift_gpu_->GetFeatureNum();

        //allocate memory
        keys.resize(num);    descriptors.resize(128 * num);

        //reading back feature vectors is faster than writing files
        //if you dont need keys or descriptors, just put NULLs here
        sift_gpu_->GetFeatureVector(&keys[0], &descriptors[0]);
        //this can be used to write your own sift file. 

        

        for (std::size_t i=0; i<keys.size(); i++)
        {
            Descriptor<unsigned char, 128> descr;
            SiftGPU::SiftKeypoint k =keys[i];

            // Feature masking
            if (mask)
            {
                const image::Image<unsigned char>& maskIma = *mask;
                if (maskIma(k.y, k.x) == 0)
                    continue;
            }
            // Create the SIFT region
            {
                for (std::size_t j = 0; j < 128; j++)
                {
                  // Cast float to unsigned char (32 bits ==> 8 bits)
                  const float d = std::round(512.0f*descriptors[i * 128 + j]);
                  descr[j] = cv::saturate_cast<unsigned char>(d);
                }
                regions->Descriptors().emplace_back(descr);
                regions->Features().emplace_back(k.x, k.y, k.s, k.o);
            }
        }
    }

    return regions;
  };

  std::unique_ptr<Regions> Allocate() const override
  {
    return std::unique_ptr<Regions_type>(new Regions_type);
  }

  template<class Archive>
  inline void serialize( Archive & ar );

  std::unique_ptr<Regions> Describe(
    const image::Image<unsigned char>& image,
    const image::Image<unsigned char>* mask = nullptr
  ) override
  {
    return Describe_GpuSift(image, mask);
  }

 private:
  Params params_;
  SiftGPU* sift_gpu_;
};

} // namespace features
} // namespace openMVG


#endif // OPENMVG_FEATURES_SIFT_GPUSIFT_IMAGE_DESCRIBER_HPP
