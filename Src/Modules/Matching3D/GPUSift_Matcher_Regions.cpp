
#include "GPUSift_Matcher_Regions.hpp"

//#include "openMVG/matching/cascade_hasher.hpp"
#include "openMVG/features/feature.hpp"
#include "openMVG/matching/matching_filters.hpp"
#include "openMVG/matching/indMatchDecoratorXY.hpp"
#include "openMVG/sfm/pipelines/sfm_regions_provider.hpp"
#include "openMVG/system/logger.hpp"
#include "openMVG/system/progressinterface.hpp"
#include "openMVG/types.hpp"
#include "SiftGPU.h"


namespace openMVG {
namespace matching_image_collection {

using namespace openMVG::matching;
using namespace openMVG::features;

GpuSift_Matcher_Regions
::GpuSift_Matcher_Regions
(
  float distRatio
):Matcher(), f_dist_ratio_(distRatio)
{
}

namespace gpu_impl
{
template <typename ScalarT>
void Match
(
  const sfm::Regions_Provider & regions_provider,
  const Pair_Set & pairs,
  float fDistRatio,
  PairWiseMatchesContainer & map_PutativeMatches, // the pairwise photometric corresponding points
  system::ProgressInterface * my_progress_bar
)
{
  if (!my_progress_bar)
    my_progress_bar = &system::ProgressInterface::dummy();

  my_progress_bar->Restart(pairs.size(), "- Matching -");

  // Init matcher
  static const int max_matches = 4096*4;
  SiftMatchGPU matcher(max_matches);

  if (matcher.VerifyContextGL() < 0)
      return;
  matcher.SetMaxSift(max_matches); // needed ?

  // Sort pairs according the first index to minimize later memory swapping
  using Map_vectorT = std::map<IndexT, std::vector<IndexT>>;
  Map_vectorT map_Pairs;
  for (const auto & pair_idx : pairs)
  {
    map_Pairs[pair_idx.first].push_back(pair_idx.second);
  }

  // Perform matching between all the pairs
  uint32_t(*match_buf)[2] = new uint32_t[max_matches][2];

  for (const auto & pair_it : map_Pairs)
  {
    if (my_progress_bar->hasBeenCanceled())
      break;

    const IndexT I = pair_it.first;
    const std::vector<IndexT> & indexToCompare = pair_it.second;

    const std::shared_ptr<features::Regions> regionsI = regions_provider.get(I);
    if (regionsI->RegionCount() == 0)
    {
      (*my_progress_bar) += indexToCompare.size();
      continue;
    }

    // Get features points and descriptors
    // const std::vector<features::PointFeature> pointFeaturesI = regionsI->GetRegionsPositions();
    const ScalarT * tabI =
      reinterpret_cast<const ScalarT*>(regionsI->DescriptorRawData());
    // const size_t dimension = regionsI->DescriptorLength();
    const size_t desc_size_I = regionsI->RegionCount();

    // set image I descriptors
    matcher.SetDescriptors(0, desc_size_I, (unsigned char*)tabI); //image I (only support unsigned char for now)

    // Loop on potential matching images J
    for (int j = 0; j < (int)indexToCompare.size(); ++j)
    {
      if (my_progress_bar->hasBeenCanceled())
        continue;

      const size_t J = indexToCompare[j];
      const std::shared_ptr<features::Regions> regionsJ = regions_provider.get(J);

      if (regionsI->Type_id() != regionsJ->Type_id())
      {
        ++(*my_progress_bar);
        continue;
      }

      const ScalarT * tabJ = reinterpret_cast<const ScalarT*>(regionsJ->DescriptorRawData());

      // set image J descriptors
      const size_t desc_size_J = regionsJ->RegionCount();
      matcher.SetDescriptors(1, desc_size_J, (unsigned char *)tabJ); //image J (only support unsigned char for now)


      //match and get result.    
      const int num_match = matcher.GetSiftMatch(max_matches, match_buf, 0.7, fDistRatio); // should we put this 0.7 as a param ?

      if (num_match < 0)
      {
        ++(*my_progress_bar);
        std::cout << " no sift matches were found;\n";
        continue;
      }
      else
        std::cout << num_match << " sift matches were found;\n";

      matching::IndMatches vec_putative_matches;
      vec_putative_matches.reserve(num_match);

      for (int k=0; k < num_match; ++k)
      {
        vec_putative_matches.emplace_back(match_buf[k][0], match_buf[k][1]);
      }

      if (!vec_putative_matches.empty())
      {
        map_PutativeMatches.insert(
          {
          {I,J},
          std::move(vec_putative_matches)
          });
      }

      ++(*my_progress_bar);
    }
  }
  delete[] match_buf;
  // delete matcher;
}
} // namespace gpu_impl

void GpuSift_Matcher_Regions::Match
(
  const std::shared_ptr<sfm::Regions_Provider> & regions_provider,
  const Pair_Set & pairs,
  PairWiseMatchesContainer & map_PutativeMatches, // the pairwise photometric corresponding points
  system::ProgressInterface * my_progress_bar
)const
{

  if (!regions_provider)
    return;

  if (regions_provider->IsBinary())
    return;

  if (regions_provider->Type_id() == typeid(unsigned char).name())
  {
    gpu_impl::Match<unsigned char>(
      *regions_provider.get(),
      pairs,
      f_dist_ratio_,
      map_PutativeMatches,
      my_progress_bar);
  }
  else
  if (regions_provider->Type_id() == typeid(float).name())
  {
    gpu_impl::Match<float>(
      *regions_provider.get(),
      pairs,
      f_dist_ratio_,
      map_PutativeMatches,
      my_progress_bar);
  }
  else
  {
    OPENMVG_LOG_ERROR << "Matcher not implemented for this region type: " << regions_provider->Type_id();
  }
}

} // namespace openMVG
} // namespace matching_image_collection
