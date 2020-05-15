add_library(DTPictureFileSetImageProvider SHARED src/DTPictureFileSetImageProvider.cpp)

set_target_properties(DTPictureFileSetImageProvider PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)

target_link_libraries( DTPictureFileSetImageProvider MatisseCommon BasicProcessing ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES})

add_custom_command(
 TARGET DTPictureFileSetImageProvider POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/DTPictureFileSetImageProvider.${D_EXT} ${MATISSE_RUNTIME_IN_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(DTPictureFileSetImageProvider
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
