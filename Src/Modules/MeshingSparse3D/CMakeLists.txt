add_library(MeshingSparse3D SHARED src/MeshingSparse3D.cpp)

target_link_libraries( MeshingSparse3D MatisseCommon BasicProcessing OpticalMapping ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES} GDAL::GDAL ${PROJ_LIBS})

# state that anybody linking to us needs to include the current source dir
target_include_directories(MeshingSparse3D
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
