add_library(Matching3D SHARED
    matching_3d.cpp
	GPUSift_Matcher_Regions.cpp
    )

include_directories(${OPENMVG_INSTALL_PATH}/include/openMVG_dependencies)

if(MSVC)
target_link_libraries(Matching3D 
	MatisseCore
	MatisseImage
	BasicProcessing
	OpticalMapping
	SiftGpu
	${OpenCV_LIBS}
	Qt5::Core 
	Qt5::Xml 
	Qt5::Widgets 
	${QUAZIP_LIBRARIES} 
	${GDAL_LIBRARY} 
	OpenMVG::openMVG_system
	OpenMVG::openMVG_image
	OpenMVG::openMVG_features
	OpenMVG::openMVG_sfm
	OpenMVG::openMVG_exif
	OpenMVG::openMVG_stlplus
	OpenMVG::openMVG_multiview
	OpenMVG::openMVG_matching_image_collection
	unofficial::vlfeat::vl
	)
else(MSVC)
target_link_libraries(Matching3D 
	MatisseCore
	MatisseImage
	BasicProcessing
	OpticalMapping
	SiftGpu
	${OpenCV_LIBS}
	Qt5::Core 
	Qt5::Xml 
	Qt5::Widgets 
	${QUAZIP_LIBRARIES} 
	${GDAL_LIBRARY} 
	OpenMVG::openMVG_system
	OpenMVG::openMVG_image
	OpenMVG::openMVG_features
	OpenMVG::openMVG_sfm
	OpenMVG::openMVG_exif
	OpenMVG::openMVG_stlplus
	OpenMVG::openMVG_multiview
	OpenMVG::openMVG_matching_image_collection
	)
endif(MSVC)



# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(Matching3D PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(Matching3D PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(Matching3D PROPERTIES PREFIX "")

add_custom_command(
 TARGET Matching3D POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/Matching3D.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(Matching3D
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
