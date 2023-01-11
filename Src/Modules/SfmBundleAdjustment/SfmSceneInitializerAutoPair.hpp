
#ifndef OPENMVG_SFM_SCENE_INITIALIZER_AUTO_PAIR_HPP
#define OPENMVG_SFM_SCENE_INITIALIZER_AUTO_PAIR_HPP

#include "openMVG/sfm/pipelines/sequential/SfmSceneInitializer.hpp"
#include "openMVG/tracks/tracks.hpp"

namespace openMVG {
namespace sfm {

// Initialize a sfm_data with a pair of camera pose.
// It sorts the pairs according the number of match correspondences and
// keep the first pair that provides a valid pair of poses.
class SfMSceneInitializerAutoPair : public SfMSceneInitializer
{
public:
  SfMSceneInitializerAutoPair(
    SfM_Data & sfm_data,
    const Features_Provider * features_provider,
    const Matches_Provider * matches_provider);

  ~SfMSceneInitializerAutoPair() override = default;

  bool Process() override;
private:
  bool InitLandmarkTracks();

  // Temporary data
  openMVG::tracks::STLMAPTracks map_tracks_; // putative landmark tracks (visibility per 3D point)

  // Helper to compute if some image have some track in common
  std::unique_ptr<openMVG::tracks::SharedTrackVisibilityHelper> shared_track_visibility_helper_;
};

} // namespace sfm
} // namespace openMVG

#endif // OPENMVG_SFM_SCENE_INITIALIZER_AUTO_PAIR_HPP
