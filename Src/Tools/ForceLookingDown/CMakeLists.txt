add_executable(force_looking_down force_looking_down_gui.ui force_looking_down_gui.cpp main.cpp)

set_target_properties(force_looking_down PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)

target_link_libraries(force_looking_down
  PRIVATE
    OpenMVG::openMVG_camera
    OpenMVG::openMVG_exif
    OpenMVG::openMVG_features
	OpenMVG::openMVG_stlplus
    OpenMVG::openMVG_geodesy
    OpenMVG::openMVG_geometry
    OpenMVG::openMVG_graph
    OpenMVG::openMVG_image
    OpenMVG::openMVG_linearProgramming
    OpenMVG::openMVG_matching
    OpenMVG::openMVG_matching_image_collection
    OpenMVG::openMVG_multiview
    OpenMVG::openMVG_numeric
    OpenMVG::openMVG_robust_estimation
    OpenMVG::openMVG_sfm
    OpenMVG::openMVG_system
	Qt5::Widgets
)

