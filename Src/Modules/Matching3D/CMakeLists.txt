add_library(Matching3D SHARED src/Matching3D.cpp)

include_directories(${OPENMVG_INSTALL_PATH}/include/openMVG_dependencies)

target_link_libraries( Matching3D 
	MatisseCommon
	BasicProcessing
	OpticalMapping
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
	OpenMVG::vlsift
    OpenMVG::openMVG_sfm
    OpenMVG::openMVG_exif
	OpenMVG::openMVG_stlplus
    OpenMVG::openMVG_multiview
	OpenMVG::openMVG_matching_image_collection)

set_target_properties(Matching3D PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)

add_custom_command(
 TARGET Matching3D POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/Matching3D.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(Matching3D
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )