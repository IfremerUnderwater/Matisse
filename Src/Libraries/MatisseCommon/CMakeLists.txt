add_library(MatisseCommon   
    src/Processor.cpp
    src/NavInfo.cpp
    src/ImageProvider.cpp
    src/Dim2FileReader.cpp
    src/PictureFileSet.cpp
    src/Image.cpp
    src/NavImage.cpp
    src/ImageSet.cpp
    src/Dim2.cpp
    src/FileImage.cpp
    src/MatisseParameters.cpp
    src/RasterProvider.cpp
    src/LifecycleComponent.cpp
    src/Dim2UdpListener.cpp
    src/GeoTransform.cpp
    src/RasterGeoreferencer.cpp
    src/FileUtils.cpp
    src/StringUtils.cpp
    src/GraphicalCharter.cpp
    src/gdal_translate_wrapper.cpp)

include_directories(${QUAZIP_INCLUDE_DIRS})

target_link_libraries( MatisseCommon BasicProcessing ${OpenCV_LIBS} Qt5::Core Qt5::Widgets Qt5::Network Qt5::Xml ${QUAZIP_LIBRARIES} GDAL::GDAL ${PROJ_LIBS})

# state that anybody linking to us needs to include the current source dir
target_include_directories(MatisseCommon
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
