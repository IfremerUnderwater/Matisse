add_library(DTPictureFileSetImageProvider SHARED src/DTPictureFileSetImageProvider.cpp)

target_link_libraries( DTPictureFileSetImageProvider MatisseCommon BasicProcessing ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES})

# state that anybody linking to us needs to include the current source dir
target_include_directories(DTPictureFileSetImageProvider
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
