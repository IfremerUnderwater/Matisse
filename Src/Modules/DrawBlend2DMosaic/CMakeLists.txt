add_library(DrawBlend2DMosaic SHARED src/DrawBlend2DMosaic.cpp)

target_link_libraries( DrawBlend2DMosaic MatisseCommon BasicProcessing OpticalMapping ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES} GDAL::GDAL ${PROJ_LIBS})

# state that anybody linking to us needs to include the current source dir
target_include_directories(DrawBlend2DMosaic
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
