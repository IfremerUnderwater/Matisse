add_library(OpticalMapping    
    mosaic_drawer.cpp
    mosaic_descriptor.cpp
    projective_camera.cpp
    file_img_exposure_compensate.cpp
    )

include_directories(${QUAZIP_INCLUDE_DIRS})

#target_link_libraries( OpticalMapping BasicProcessing MatisseTools MatisseCommon ${OpenCV_LIBS} Qt5::Core Qt5::Network Qt5::Xml Qt5::XmlPatterns Qt5::Script ${QUAZIP_LIBRARIES})
target_link_libraries( OpticalMapping BasicProcessing MatisseCore NavTools SystemTools ${OpenCV_LIBS} Qt5::Core Qt5::Network Qt5::Xml Qt5::XmlPatterns ${QUAZIP_LIBRARIES} ${GeographicLib_LIBRARIES})


# state that anybody linking to us needs to include the current source dir
target_include_directories(OpticalMapping
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
