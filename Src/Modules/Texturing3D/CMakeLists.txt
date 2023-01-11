add_library(Texturing3D SHARED
    texturing_3d.cpp
    )

target_link_libraries(Texturing3D
	MatisseCore
	MatisseImage
	BasicProcessing
	OpticalMapping
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
	OpenMVG::openMVG_stlplus
	)

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(Texturing3D PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(Texturing3D PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(Texturing3D PROPERTIES PREFIX "")


add_custom_command(
 TARGET Texturing3D POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/Texturing3D.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(Texturing3D
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
