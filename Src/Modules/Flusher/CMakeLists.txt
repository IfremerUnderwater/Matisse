add_library(Flusher SHARED src/Flusher.cpp)

target_link_libraries( Flusher MatisseCommon BasicProcessing ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES} GDAL::GDAL ${PROJ_LIBS})

# state that anybody linking to us needs to include the current source dir
target_include_directories(Flusher
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
