add_library(Flusher SHARED
    flusher.cpp
    )

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(Flusher PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(Flusher PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(Flusher PROPERTIES PREFIX "")

target_link_libraries(Flusher 
	MatisseCore 
	NavTools 
	BasicProcessing 
	${OpenCV_LIBS} 
	Qt5::Core 
	Qt5::Xml 
	Qt5::Widgets 
	${QUAZIP_LIBRARIES} 
	GDAL::GDAL 
	${PROJ_LIBS}
	)

add_custom_command(
 TARGET Flusher POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/Flusher.${D_EXT} ${MATISSE_RUNTIME_IN_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(Flusher
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
