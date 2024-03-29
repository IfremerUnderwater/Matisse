#ifndef OPENMVG_MATCHING_GPUSIFT_MATCHER_REGIONS_HPP
#define OPENMVG_MATCHING_GPUSIFT_MATCHER_REGIONS_HPP

#include <memory>

#include "openMVG/matching_image_collection/Matcher.hpp"

namespace openMVG { namespace matching { class PairWiseMatchesContainer; } }
namespace openMVG { namespace sfm { struct Regions_Provider; } }

namespace openMVG {
namespace matching_image_collection {

/// Implementation of an Image Collection Matcher
/// Compute putative matches between a collection of pictures
/// Spurious correspondences are discarded by using the
///  a threshold over the distance ratio of the 2 nearest neighbours.
/// Using a Cascade Hashing matching
/// Cascade hashing tables are computed once and used for all the regions.
///
class GpuSift_Matcher_Regions : public Matcher
{
  public:
  explicit GpuSift_Matcher_Regions
  (
    float dist_ratio
  );

  /// Find corresponding points between some pair of view Ids
  void Match
  (const std::shared_ptr<sfm::Regions_Provider> & regions_provider,
    const Pair_Set & pairs,
    matching::PairWiseMatchesContainer & map_PutativeMatches, // the pairwise photometric corresponding points
    system::ProgressInterface * progress = nullptr
  ) const override;

  private:
  // Distance ratio used to discard spurious correspondence
  float f_dist_ratio_;
};

} // namespace matching_image_collection
} // namespace openMVG

#endif // OPENMVG_MATCHING_GPUSIFT_MATCHER_REGIONS_HPP
