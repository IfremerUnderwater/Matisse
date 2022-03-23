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
    uint32_t iteration = 1024
  ):
    m_f_filter(dPrecision, iteration),
    m_h_filter(dPrecision, iteration),
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

    std::cout << "\n Inliers : (H)" << num_h_inliers << " / (F) " << num_f_inliers << " out of " << vec_PutativeMatches.size() << " matches!";
      
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
    if (m_HMatrix_is_best)
      success = m_h_filter.Geometry_guided_matching(sfm_data, regions_provider, pairIndex, 
                                      dDistanceRatio, matches);
    else
      success = m_f_filter.Geometry_guided_matching(sfm_data, regions_provider, pairIndex, 
                                      dDistanceRatio, matches);

    std::cout << "\n New number of matches : " << matches.size() << "\n";

    return success;
  }

  GeometricFilter_FMatrix_AC m_f_filter;
  GeometricFilter_HMatrix_AC m_h_filter;
  bool m_HMatrix_is_best;
};

} // namespace matching_image_collection
} // namespace openMVG

#endif // OPENMVG_MATCHING_IMAGE_COLLECTION_H_F_AC_ROBUST_HPP
