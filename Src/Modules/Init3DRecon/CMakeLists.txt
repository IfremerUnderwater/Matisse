add_library(Init3DRecon SHARED src/Init3DRecon.cpp)

target_link_libraries( Init3DRecon PRIVATE
    MatisseCommon
    BasicProcessing
    OpticalMapping
    ${OpenCV_LIBS}
    Qt5::Core
    Qt5::Xml
    Qt5::Widgets
    ${QUAZIP_LIBRARIES}
    GDAL::GDAL
    ${PROJ_LIBS}
    GeographicLib::GeographicLib
	OpenMVG::openMVG_system
	OpenMVG::openMVG_image
    OpenMVG::openMVG_features
    OpenMVG::openMVG_sfm
    OpenMVG::openMVG_exif )

# state that anybody linking to us needs to include the current source dir
target_include_directories(Init3DRecon
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
