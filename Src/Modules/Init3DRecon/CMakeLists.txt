add_library(Init3DRecon SHARED
    init_3d_recon.cpp
    )

target_link_libraries(Init3DRecon PRIVATE
    MatisseCore
    MatisseImage
    NavTools
    BasicProcessing
    OpticalMapping
    ${OpenCV_LIBS}
    Qt5::Core
    Qt5::Xml
    Qt5::Widgets
    ${QUAZIP_LIBRARIES}
    GDAL::GDAL
    ${PROJ_LIBS}
    ${GeographicLib_LIBRARIES}
	OpenMVG::openMVG_system
	OpenMVG::openMVG_image
    OpenMVG::openMVG_features
    OpenMVG::openMVG_sfm
    OpenMVG::openMVG_exif
	OpenMVG::openMVG_stlplus
	OpenMVG::vlsift
	)

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(Init3DRecon PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(Init3DRecon PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(Init3DRecon PROPERTIES PREFIX "")

add_custom_command(
 TARGET Init3DRecon POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/Init3DRecon.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(Init3DRecon
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
