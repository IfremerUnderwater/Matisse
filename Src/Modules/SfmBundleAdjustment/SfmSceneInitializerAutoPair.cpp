//#define OPENMVG_STD_UNORDERED_MAP

#include "SfmSceneInitializerAutoPair.hpp"

#include "openMVG/cameras/Camera_Intrinsics.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_matches_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_robust_model_estimation.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/stl/stl.hpp"
#include "openMVG/system/logger.hpp"


namespace openMVG {

using namespace cameras;
using namespace geometry;
using namespace matching;

namespace sfm {

SfMSceneInitializerAutoPair::SfMSceneInitializerAutoPair(
  SfM_Data & sfm_data,
  const Features_Provider * features_provider,
  const Matches_Provider * matches_provider)
  :SfMSceneInitializer(sfm_data, features_provider, matches_provider)
{
  sfm_data_.poses.clear();
}

bool SfMSceneInitializerAutoPair::Process()
{
  if (sfm_data_.GetIntrinsics().empty())
    return false;

  if (!InitLandmarkTracks())
      return false;

  // select a pair that have the largest baseline (mean angle between its bearing vectors).

  const unsigned iMin_inliers_count = 100;
  const float fRequired_min_angle = 3.0f;
  const float fLimit_max_angle = 60.0f; // More than 60 degree, we cannot rely on matches for initial pair seeding

    // List Views that support valid intrinsic (view that could be used for Essential matrix computation)
  std::set<IndexT> valid_views;
  for (Views::const_iterator it = sfm_data_.GetViews().begin();
      it != sfm_data_.GetViews().end(); ++it)
  {
      const View* v = it->second.get();
      if (sfm_data_.GetIntrinsics().count(v->id_intrinsic))
          valid_views.insert(v->id_view);
  }

  if (valid_views.size() < 2)
  {
      return false; // There is not view that support valid intrinsic data
  }

  std::vector<std::pair<double, Pair>> scoring_per_pair;

  // Compute the relative pose & the 'baseline score'
  system::LoggerProgress my_progress_bar(matches_provider_->pairWise_matches_.size(),
      "Selection of an initial pair");

#pragma omp parallel
  for (const std::pair<Pair, IndMatches>& match_pair : matches_provider_->pairWise_matches_)
  {
#pragma omp single nowait
      {
          ++my_progress_bar;

          const Pair current_pair = match_pair.first;

          const uint32_t I = std::min(current_pair.first, current_pair.second);
          const uint32_t J = std::max(current_pair.first, current_pair.second);
          if (valid_views.count(I) && valid_views.count(J))
          {
              const View
                  * view_I = sfm_data_.GetViews().at(I).get(),
                  * view_J = sfm_data_.GetViews().at(J).get();
              const Intrinsics::const_iterator
                  iterIntrinsic_I = sfm_data_.GetIntrinsics().find(view_I->id_intrinsic),
                  iterIntrinsic_J = sfm_data_.GetIntrinsics().find(view_J->id_intrinsic);

              const auto
                  cam_I = iterIntrinsic_I->second.get(),
                  cam_J = iterIntrinsic_J->second.get();
              if (cam_I && cam_J)
              {
                  openMVG::tracks::STLMAPTracks map_tracksCommon;
                  shared_track_visibility_helper_->GetTracksInImages({ I, J }, map_tracksCommon);

                  // Copy points correspondences to arrays for relative pose estimation
                  const size_t n = map_tracksCommon.size();
                  Mat xI(2, n), xJ(2, n);
                  size_t cptIndex = 0;
                  for (const auto& track_iter : map_tracksCommon)
                  {
                      auto iter = track_iter.second.cbegin();
                      const uint32_t i = iter->second;
                      const uint32_t j = (++iter)->second;

                      Vec2 feat = features_provider_->feats_per_view.at(I)[i].coords().cast<double>();
                      xI.col(cptIndex) = cam_I->get_ud_pixel(feat);
                      feat = features_provider_->feats_per_view.at(J)[j].coords().cast<double>();
                      xJ.col(cptIndex) = cam_J->get_ud_pixel(feat);
                      ++cptIndex;
                  }

                  // Robust estimation of the relative pose
                  RelativePose_Info relativePose_info;
                  relativePose_info.initial_residual_tolerance = Square(4.0);

                  if (robustRelativePose(
                      cam_I, cam_J,
                      xI, xJ, relativePose_info,
                      { cam_I->w(), cam_I->h() }, { cam_J->w(), cam_J->h() },
                      256)
                      && relativePose_info.vec_inliers.size() > iMin_inliers_count)
                  {
                      // Triangulate inliers & compute angle between bearing vectors
                      std::vector<float> vec_angles;
                      vec_angles.reserve(relativePose_info.vec_inliers.size());
                      const Pose3 pose_I = Pose3(Mat3::Identity(), Vec3::Zero());
                      const Pose3 pose_J = relativePose_info.relativePose;
                      for (const uint32_t& inlier_idx : relativePose_info.vec_inliers)
                      {
                          openMVG::tracks::STLMAPTracks::const_iterator iterT = map_tracksCommon.begin();
                          std::advance(iterT, inlier_idx);
                          tracks::submapTrack::const_iterator iter = iterT->second.begin();
                          const Vec2 featI = features_provider_->feats_per_view.at(I)[iter->second].coords().cast<double>();
                          const Vec2 featJ = features_provider_->feats_per_view.at(J)[(++iter)->second].coords().cast<double>();
                          vec_angles.push_back(AngleBetweenRay(pose_I, cam_I, pose_J, cam_J,
                              cam_I->get_ud_pixel(featI), cam_J->get_ud_pixel(featJ)));
                      }
                      // Compute the median triangulation angle
                      const unsigned median_index = vec_angles.size() / 2;
                      std::nth_element(
                          vec_angles.begin(),
                          vec_angles.begin() + median_index,
                          vec_angles.end());
                      const float scoring_angle = vec_angles[median_index];
                      // Store the pair iff the pair is in the asked angle range [fRequired_min_angle;fLimit_max_angle]
                      if (scoring_angle > fRequired_min_angle &&
                          scoring_angle < fLimit_max_angle)
                      {
#pragma omp critical
                          scoring_per_pair.emplace_back(scoring_angle, current_pair);
                      }
                  }
              }
          }
      } // omp section
  }

  std::sort(scoring_per_pair.begin(), scoring_per_pair.end());
  // Since scoring is ordered in increasing order, reverse the order
  std::reverse(scoring_per_pair.begin(), scoring_per_pair.end());
  if (!scoring_per_pair.empty())
  {
      // Get best pair
      const Pair best_pair = scoring_per_pair.begin()->second;

      const uint32_t I = std::min(best_pair.first, best_pair.second);
      const uint32_t J = std::max(best_pair.first, best_pair.second);

      if (valid_views.count(I) && valid_views.count(J))
      {
          const View
              * view_I = sfm_data_.GetViews().at(I).get(),
              * view_J = sfm_data_.GetViews().at(J).get();
          const Intrinsics::const_iterator
              iterIntrinsic_I = sfm_data_.GetIntrinsics().find(view_I->id_intrinsic),
              iterIntrinsic_J = sfm_data_.GetIntrinsics().find(view_J->id_intrinsic);

          const auto
              cam_I = iterIntrinsic_I->second.get(),
              cam_J = iterIntrinsic_J->second.get();
          if (cam_I && cam_J)
          {
              openMVG::tracks::STLMAPTracks map_tracksCommon;
              shared_track_visibility_helper_->GetTracksInImages({ I, J }, map_tracksCommon);

              // Copy points correspondences to arrays for relative pose estimation
              const size_t n = map_tracksCommon.size();
              Mat xI(2, n), xJ(2, n);
              size_t cptIndex = 0;
              for (const auto& track_iter : map_tracksCommon)
              {
                  auto iter = track_iter.second.cbegin();
                  const uint32_t i = iter->second;
                  const uint32_t j = (++iter)->second;

                  Vec2 feat = features_provider_->feats_per_view.at(I)[i].coords().cast<double>();
                  xI.col(cptIndex) = cam_I->get_ud_pixel(feat);
                  feat = features_provider_->feats_per_view.at(J)[j].coords().cast<double>();
                  xJ.col(cptIndex) = cam_J->get_ud_pixel(feat);
                  ++cptIndex;
              }

              // Robust estimation of the relative pose
              RelativePose_Info relativePose_info;
              relativePose_info.initial_residual_tolerance = Square(4.0);

              if (robustRelativePose(
                  cam_I, cam_J,
                  xI, xJ, relativePose_info,
                  { cam_I->w(), cam_I->h() }, { cam_J->w(), cam_J->h() },
                  256)
                  && relativePose_info.vec_inliers.size() > iMin_inliers_count)
              {
                sfm_data_.poses[view_I->id_pose] = Pose3(Mat3::Identity(), Vec3::Zero());
                sfm_data_.poses[view_J->id_pose] = relativePose_info.relativePose;
                return true;
              }
          }
      }

      return false;
  }
  return false;
  //// Code below is max_pair code (to be removed)
  ////
  //// Sort the PairWiseMatches by matches count.
  //// Keep the first pair that provides a valid relative pose.
  ////
  //std::vector<IndexT> matches_count_per_pair;
  //matches_count_per_pair.reserve(matches_provider_->pairWise_matches_.size());
  //std::transform(matches_provider_->pairWise_matches_.cbegin(), matches_provider_->pairWise_matches_.cend(),
  //  std::back_inserter(matches_count_per_pair),
  //  [](const std::pair<Pair, IndMatches> & match_pair) -> IndexT { return match_pair.second.size(); });

  //// sort the Pairs in descending order according their matches count
  //using namespace stl::indexed_sort;
  //std::vector<sort_index_packet_descend<IndexT, IndexT>> packet_vec(matches_count_per_pair.size());
  //sort_index_helper(packet_vec, &matches_count_per_pair[0], std::min((IndexT)10, (IndexT)matches_count_per_pair.size()));

  //// Iterate through the pairs and try to find the relative pose
  //for (size_t i = 0; i < matches_count_per_pair.size(); ++i)
  //{
  //  const IndexT index = packet_vec[i].index;
  //  openMVG::matching::PairWiseMatches::const_iterator iter = matches_provider_->pairWise_matches_.cbegin();
  //  std::advance(iter, index);

  //  OPENMVG_LOG_INFO << "(" << iter->first.first << "," << iter->first.second <<"): "
  //    << iter->second.size() << " matches" << std::endl;

  //  const IndexT
  //    I = iter->first.first,
  //    J = iter->first.second;

  //  const View
  //    * view_I = sfm_data_.views[I].get(),
  //    * view_J = sfm_data_.views[J].get();

  //  // Check that the pair has valid intrinsic
  //  if (sfm_data_.GetIntrinsics().count(view_I->id_intrinsic) == 0 ||
  //      sfm_data_.GetIntrinsics().count(view_J->id_intrinsic) == 0)
  //    continue;

  //  const IntrinsicBase
  //    * cam_I = sfm_data_.GetIntrinsics().at(view_I->id_intrinsic).get(),
  //    * cam_J = sfm_data_.GetIntrinsics().at(view_J->id_intrinsic).get();

  //  // Compute for each feature the un-distorted camera coordinates
  //  const matching::IndMatches & matches = matches_provider_->pairWise_matches_.at(iter->first);
  //  size_t number_matches = matches.size();
  //  Mat2X x1(2, number_matches), x2(2, number_matches);
  //  number_matches = 0;
  //  for (const auto & match : matches)
  //  {
  //    x1.col(number_matches) = cam_I->get_ud_pixel(features_provider_->feats_per_view.at(I)[match.i_].coords().cast<double>());
  //    x2.col(number_matches) = cam_J->get_ud_pixel(features_provider_->feats_per_view.at(J)[match.j_].coords().cast<double>());
  //    ++number_matches;
  //  }

  //  RelativePose_Info relativePose_info;
  //  relativePose_info.initial_residual_tolerance = Square(2.5);
  //  if (!robustRelativePose(cam_I, cam_J,
  //                          x1, x2, relativePose_info,
  //                          {cam_I->w(), cam_I->h()},
  //                          {cam_J->w(), cam_J->h()},
  //                          2048))
  //  {
  //    continue;
  //  }
  //  sfm_data_.poses[view_I->id_pose] = Pose3(Mat3::Identity(), Vec3::Zero());
  //  sfm_data_.poses[view_J->id_pose] = relativePose_info.relativePose;
  //  return true;
  //}
  //return false;
}

bool SfMSceneInitializerAutoPair::InitLandmarkTracks()
{
    // Compute tracks from matches
    tracks::TracksBuilder tracksBuilder;

    {
        // List of features matches for each couple of images
        const openMVG::matching::PairWiseMatches& map_Matches = matches_provider_->pairWise_matches_;
        OPENMVG_LOG_INFO << "Track building";

        tracksBuilder.Build(map_Matches);
        OPENMVG_LOG_INFO << "Track filtering";
        tracksBuilder.Filter();
        OPENMVG_LOG_INFO << "Track export to internal struct";
        //-- Build tracks with STL compliant type :
        tracksBuilder.ExportToSTL(map_tracks_);

        {
            std::ostringstream osTrack;
            //-- Display stats :
            //    - number of images
            //    - number of tracks
            std::set<uint32_t> set_imagesId;
            tracks::TracksUtilsMap::ImageIdInTracks(map_tracks_, set_imagesId);
            osTrack << "\n------------------\n"
                << "-- Tracks Stats --" << "\n"
                << " Tracks number: " << tracksBuilder.NbTracks() << "\n"
                << " Images Id: " << "\n";
            std::copy(set_imagesId.begin(),
                set_imagesId.end(),
                std::ostream_iterator<uint32_t>(osTrack, ", "));
            osTrack << "\n------------------\n";

            std::map<uint32_t, uint32_t> map_Occurrence_TrackLength;
            tracks::TracksUtilsMap::TracksLength(map_tracks_, map_Occurrence_TrackLength);
            osTrack << "TrackLength, Occurrence" << "\n";
            for (const auto& it : map_Occurrence_TrackLength) {
                osTrack << "\t" << it.first << "\t" << it.second << "\n";
            }
            OPENMVG_LOG_INFO << osTrack.str();
        }
    }
    // Initialize the shared track visibility helper
    shared_track_visibility_helper_.reset(new openMVG::tracks::SharedTrackVisibilityHelper(map_tracks_));
    return map_tracks_.size() > 0;
}

} // namespace sfm
} // namespace openMVG
