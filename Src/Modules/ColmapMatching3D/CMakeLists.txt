add_library(ColmapMatching3D SHARED
    colmap_matching_3d.cpp
    )

include_directories(${OPENMVG_INSTALL_PATH}/include/openMVG_dependencies)


target_link_libraries(ColmapMatching3D 
PRIVATE
	MatisseCore
	MatisseImage
	NavTools
	Qt5::Core 
	Qt5::Xml 
	Qt5::Widgets 
	colmap::colmap
	)



# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(ColmapMatching3D PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(ColmapMatching3D PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(ColmapMatching3D PROPERTIES PREFIX "")

add_custom_command(
 TARGET ColmapMatching3D POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/ColmapMatching3D.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(ColmapMatching3D
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
