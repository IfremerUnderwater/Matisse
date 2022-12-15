// This file is part of OpenMVG, an Open Multiple View Geometry C++ library.

// Copyright (c) 2014, 2015 Pierre MOULON.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENMVG_MATCHING_IMAGE_COLLECTION_H_F_AC_ROBUST_HPP
#define OPENMVG_MATCHING_IMAGE_COLLECTION_H_F_AC_ROBUST_HPP

#include <limits>
#include <utility>
#include <vector>
#include <mutex>

#include "openMVG/matching/indMatch.hpp"
#include "openMVG/matching/indMatchDecoratorXY.hpp"
#include "openMVG/matching_image_collection/Geometric_Filter_utils.hpp"
#include "openMVG/multiview/solver_homography_kernel.hpp"
#include "openMVG/robust_estimation/guided_matching.hpp"
#include "openMVG/robust_estimation/robust_estimator_ACRansac.hpp"
#include "openMVG/robust_estimation/robust_estimator_ACRansacKernelAdaptator.hpp"
#include "openMVG/sfm/pipelines/sfm_regions_provider.hpp"
#include "openMVG/sfm/sfm_data.hpp"

#include "openMVG/matching_image_collection/H_ACRobust.hpp"
#include "openMVG/matching_image_collection/F_ACRobust.hpp"

#include "SiftGPU.h"

namespace openMVG {

namespace sfm {
  struct Regions_Provider;
} // namespace sfm

namespace matching_image_collection {

//-- A contrario homography matrix estimation template functor used for filter pair of putative correspondences
struct GeometricFilter_H_F_AC
{
  GeometricFilter_H_F_AC
  (
    double dPrecision = std::numeric_limits<double>::infinity(),
    uint32_t iteration = 1024,
    SiftMatchGPU* psift_gpu_matcher = nullptr
  ):
    m_f_filter(dPrecision, iteration),
    m_h_filter(dPrecision, iteration),
    m_psift_gpu_matcher(psift_gpu_matcher),
    m_HMatrix_is_best(false)
  {
  }

  /// Robust fitting of the HOMOGRAPHY matrix
  template<typename Regions_or_Features_ProviderT>
  bool Robust_estimation
  (
    const sfm::SfM_Data * sfm_data,
    const std::shared_ptr<Regions_or_Features_ProviderT> & regions_provider,
    const Pair pairIndex,
    const matching::IndMatches & vec_PutativeMatches,
    matching::IndMatches & geometric_inliers)
  {
    matching::IndMatches f_geometric_inliers;
    m_f_filter.Robust_estimation(sfm_data, regions_provider, pairIndex, vec_PutativeMatches, f_geometric_inliers);

    const size_t num_f_inliers = f_geometric_inliers.size();

    matching::IndMatches h_geometric_inliers;
    m_h_filter.Robust_estimation(sfm_data, regions_provider, pairIndex, vec_PutativeMatches, h_geometric_inliers);

    const size_t num_h_inliers = h_geometric_inliers.size();

    if (num_f_inliers == 0 && num_h_inliers == 0)
      return false;

    std::cout << "\n Inliers : (H)" << num_h_inliers << " / (F) " << num_f_inliers 
        << " out of " << vec_PutativeMatches.size() << " matches!";
      
    // Init with FMatrix result
    geometric_inliers.swap(f_geometric_inliers);

    // Switch to Homography if better (following colmap ratio)
    const float h_f_ratio = static_cast<float>(num_h_inliers) / num_f_inliers;
    if (h_f_ratio > 0.8f)
    {
      geometric_inliers.swap(h_geometric_inliers);
      m_HMatrix_is_best = true;
    }

    if (m_HMatrix_is_best)
      std::cout << "\n Homography Chosen for Robust Geometry Filtering!";
    else
      std::cout << "\n Fundamental Mat Chosen for Robust Geometry Filtering!";
    
    const double dprec_robust = m_HMatrix_is_best ? m_h_filter.m_dPrecision_robust
                                            : m_f_filter.m_dPrecision_robust;
    
    std::cout << "\n Robust Precision threshold : " << dprec_robust << "\n";

    return true;
  }

  bool Geometry_guided_matching
  (
    const sfm::SfM_Data * sfm_data,
    const std::shared_ptr<sfm::Regions_Provider> & regions_provider,
    const Pair pairIndex,
    const double dDistanceRatio,
    matching::IndMatches & matches
  )
  {
    bool success = false;

    if (m_psift_gpu_matcher != nullptr)
    {
      if (regions_provider->Type_id() == typeid(unsigned char).name())
      {
        success = Geometry_guided_matching_SiftGPU<unsigned char>(
                    sfm_data, regions_provider, pairIndex, 
                    dDistanceRatio, matches);
      }
      else if (regions_provider->Type_id() == typeid(float).name())
      {
        success = Geometry_guided_matching_SiftGPU<float>(
                    sfm_data, regions_provider, pairIndex, 
                    dDistanceRatio, matches);
      }
    }
    else if (m_HMatrix_is_best)
    {
      success = m_h_filter.Geometry_guided_matching(
                  sfm_data, regions_provider, pairIndex, 
                  dDistanceRatio, matches);
    }
    else
    {
      success = m_f_filter.Geometry_guided_matching(
                  sfm_data, regions_provider, pairIndex, 
                  dDistanceRatio, matches);
    }

    std::cout << "\n GuidedMatching - new number of matches : " << matches.size() << "\n";

    return success;
  }

  template<typename ScalarT>
  bool Geometry_guided_matching_SiftGPU
  (
    const sfm::SfM_Data * sfm_data,
    const std::shared_ptr<sfm::Regions_Provider> & regions_provider,
    const Pair pairIndex,
    const double dDistanceRatio,
    matching::IndMatches & matches
  )
  {
    // Setup Features for matching
    // ========================================================

    // Get back corresponding view index
    const IndexT iIndex = pairIndex.first;
    const IndexT jIndex = pairIndex.second;

    const std::shared_ptr<features::Regions>
      regionsI = regions_provider->get(iIndex),
      regionsJ = regions_provider->get(jIndex);
    
    // Get features points and descriptors for Image I
    // ------------------------------------------------

    // 1st Descriptor
    const ScalarT * tabI =
      reinterpret_cast<const ScalarT*>(regionsI->DescriptorRawData());
    const size_t desc_size_I = regionsI->RegionCount();
    
    // 2nd Features location (MANDATORY ORDER!)
    const std::vector<features::PointFeature> pointFeaturesI = regionsI->GetRegionsPositions();

    std::vector<SiftGPU::SiftKeypoint> vkeysI;
    vkeysI.reserve(pointFeaturesI.size());
    for (const auto &kp : pointFeaturesI)
    {
      vkeysI.push_back({kp.x(), kp.y(), 1.f, 0.f});
    }
    
    if (desc_size_I != vkeysI.size())
    {
      std::cerr << "\n Not the same amount of keypoints and descriptor for Image I! ";
      std::cerr << "Nb desc : " << desc_size_I << " / ";
      std::cerr << "Nb keypoints : " << vkeysI.size() << "\n";
      return false;
    }

    
    // Get features points and descriptors for Image J
    // ------------------------------------------------

    // 1st Descriptor
    const ScalarT * tabJ = reinterpret_cast<const ScalarT*>(regionsJ->DescriptorRawData());
    const size_t desc_size_J = regionsJ->RegionCount();
    
    // 2nd Features location (MANDATORY ORDER!)
    const std::vector<features::PointFeature> pointFeaturesJ = regionsJ->GetRegionsPositions();

    std::vector<SiftGPU::SiftKeypoint> vkeysJ;
    vkeysJ.reserve(pointFeaturesI.size());
    for (const auto &kp : pointFeaturesJ)
    {
      vkeysJ.push_back({kp.x(), kp.y(), 1.f, 0.f});
    }

    if (desc_size_J != vkeysJ.size())
    {
      std::cerr << "\n Not the same amount of keypoints and descriptor for Image J! ";
      std::cerr << "Nb desc : " << desc_size_J << " / ";
      std::cerr << "Nb keypoints : " << vkeysJ.size() << "\n";
      return false;
    }

    // Setup Homography / Fundamental Mat for Guided Matching
    // ========================================================

    Eigen::Matrix<float, 3, 3, Eigen::RowMajor> F;
    Eigen::Matrix<float, 3, 3, Eigen::RowMajor> H;
    float* F_ptr = nullptr;
    float* H_ptr = nullptr;

    if (m_HMatrix_is_best)
    {
      H = m_h_filter.m_H.cast<float>();
      H_ptr = H.data();
    }
    else 
    {
      F = m_f_filter.m_F.cast<float>();
      F_ptr = F.data();
    }
    
    // Apply Guided Matching
    // ========================================================

    // Check SiftGPU Init
    if (m_psift_gpu_matcher->VerifyContextGL() < 0)
      return false;

    const int max_matches = m_psift_gpu_matcher->GetMaxSift();

    // set image I descriptors
    m_psift_gpu_matcher->SetDescriptors(0, desc_size_I, (unsigned char*)tabI); //image I (only support unsigned char for now)
    m_psift_gpu_matcher->SetFeautreLocation(0, reinterpret_cast<const float*>(vkeysI.data()), 2); // 2 is the amount of values to skip in vkeys struct
    
    // set image J descriptors
    m_psift_gpu_matcher->SetDescriptors(1, desc_size_J, (unsigned char*)tabJ); //image J (only support unsigned char for now)
    m_psift_gpu_matcher->SetFeautreLocation(1, reinterpret_cast<const float*>(vkeysJ.data()), 2); // 2 is the amount of values to skip in vkeys struct

    // Perform matching between all the pairs
    uint32_t(*match_buf)[2] = new uint32_t[max_matches][2];

    static const int mutual_best_match = 1;

    static const float sift_max_dist = 0.7; // Suggested max distance between two Sift descriptors

    const int num_matches = 
        m_psift_gpu_matcher->GetGuidedSiftMatch(
            max_matches, 
            match_buf, 
            H_ptr, 
            F_ptr, 
            sift_max_dist, 
            static_cast<float>(dDistanceRatio), 
            static_cast<float>(Square(m_h_filter.m_dPrecision_robust)),
            static_cast<float>(Square(m_f_filter.m_dPrecision_robust)),
            mutual_best_match);

    const bool success = num_matches > 0;

    if (success)
    {
      // Recover matches
      // ====================
      matches.reserve(num_matches);
      for (int i=0; i < num_matches; ++i)
      {
        matches.emplace_back(match_buf[i][0], match_buf[i][1]);
      }
    }

    delete[] match_buf;

    return success;
  }

  GeometricFilter_FMatrix_AC m_f_filter;
  GeometricFilter_HMatrix_AC m_h_filter;
  SiftMatchGPU* m_psift_gpu_matcher;
  bool m_HMatrix_is_best;
};

} // namespace matching_image_collection
} // namespace openMVG

#endif // OPENMVG_MATCHING_IMAGE_COLLECTION_H_F_AC_ROBUST_HPP
