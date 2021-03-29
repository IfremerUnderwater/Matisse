add_library(MatisseCommon   
	)

#include_directories(${QUAZIP_INCLUDE_DIRS})

target_link_libraries( MatisseCommon SystemTools NavTools NetworkTools BasicProcessing ${OpenCV_LIBS} Qt5::Core Qt5::Widgets Qt5::Xml
     GDAL::GDAL ${PROJ_LIBS})

# state that anybody linking to us needs to include the current source dir
target_include_directories(MatisseCommon
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
