add_library(ColmapMapper SHARED
    colmap_mapper.cpp
    )

target_link_libraries(ColmapMapper
	MatisseCore
	MatisseImage
	Qt5::Core
	Qt5::Xml
	Qt5::Widgets
	colmap::colmap
	)

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(ColmapMapper PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(ColmapMapper PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(ColmapMapper PROPERTIES PREFIX "")

add_custom_command(
 TARGET ColmapMapper POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/ColmapMapper.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(ColmapMapper
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
