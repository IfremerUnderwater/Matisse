add_library(InitColmap3D SHARED
    init_colmap_3d_recon.cpp
    )

target_link_libraries(InitColmap3D PRIVATE
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
    ${GDAL_LIBRARY}
    ${GeographicLib_LIBRARIES}
    colmap::colmap
	)

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(InitColmap3D PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(InitColmap3D PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(InitColmap3D PROPERTIES PREFIX "")

add_custom_command(
 TARGET InitColmap3D POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/InitColmap3D.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(InitColmap3D
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
