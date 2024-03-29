add_library(ReconstructionLister SHARED
    reconstruction_lister.cpp
    )

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(ReconstructionLister PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(ReconstructionLister PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(ReconstructionLister PROPERTIES PREFIX "")

target_link_libraries(
	ReconstructionLister 
	MatisseCore 
	BasicProcessing 
	${OpenCV_LIBS} 
	Qt5::Core 
	Qt5::Xml 
	Qt5::Widgets 
	${QUAZIP_LIBRARIES} 
	${GDAL_LIBRARY} 
	)

add_custom_command(
 TARGET ReconstructionLister POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/ReconstructionLister.${D_EXT} ${MATISSE_RUNTIME_OUT_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(ReconstructionLister
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
