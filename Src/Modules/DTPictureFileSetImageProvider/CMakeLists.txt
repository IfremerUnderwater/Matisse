add_library(DTPictureFileSetImageProvider SHARED
    dt_picture_file_set_image_provider.cpp
    )

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(DTPictureFileSetImageProvider PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(DTPictureFileSetImageProvider PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(DTPictureFileSetImageProvider PROPERTIES PREFIX "")

include_directories(${QUAZIP_INCLUDE_DIRS})

target_link_libraries(DTPictureFileSetImageProvider 
	MatisseCore 
	NavTools 
	BasicProcessing 
	${OpenCV_LIBS} 
	Qt5::Core 
	Qt5::Xml 
	Qt5::Widgets 
	${QUAZIP_LIBRARIES}
	)

add_custom_command(
 TARGET DTPictureFileSetImageProvider POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/DTPictureFileSetImageProvider.${D_EXT} ${MATISSE_RUNTIME_IN_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(DTPictureFileSetImageProvider
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
