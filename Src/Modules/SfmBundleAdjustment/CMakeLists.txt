add_library(SfmBundleAdjustment SHARED src/SfmBundleAdjustment.cpp)

target_link_libraries( SfmBundleAdjustment
	MatisseCommon
	BasicProcessing
	${OpenCV_LIBS}
	Qt5::Core
	Qt5::Xml
	Qt5::Widgets
	${QUAZIP_LIBRARIES}
	GDAL::GDAL
	${PROJ_LIBS}
	OpenMVG::openMVG_system
	OpenMVG::openMVG_image
    OpenMVG::openMVG_features
    OpenMVG::openMVG_sfm
	OpenMVG::openMVG_stlplus
    OpenMVG::openMVG_multiview
	OpenMVG::openMVG_matching_image_collection)

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(SfmBundleAdjustment PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(SfmBundleAdjustment PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(SfmBundleAdjustment PROPERTIES PREFIX "")

add_custom_command(
 TARGET SfmBundleAdjustment POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/SfmBundleAdjustment.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(SfmBundleAdjustment
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
