add_library(BridgeModule SHARED BridgeModule.cpp)

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(BridgeModule PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(BridgeModule PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(BridgeModule PROPERTIES PREFIX "")

target_link_libraries(BridgeModule 
	MatisseCore 
	BasicProcessing 
	OpticalMapping 
	${OpenCV_LIBS} 
	Qt5::Core 
	Qt5::Xml 
	Qt5::Widgets 
	${QUAZIP_LIBRARIES} 
	GDAL::GDAL 
	${PROJ_LIBS}
	)

add_custom_command(
 TARGET BridgeModule POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/BridgeModule.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(BridgeModule
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
