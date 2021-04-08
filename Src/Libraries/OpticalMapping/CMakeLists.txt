add_library(OpticalMapping    
    MosaicDrawer.cpp
    MosaicOptimizer.cpp
    MosaicDescriptor.cpp
    ProjectiveCamera.cpp
    FileImgExposureCompensate.cpp)

include_directories(${QUAZIP_INCLUDE_DIRS})

#target_link_libraries( OpticalMapping BasicProcessing MatisseTools MatisseCommon ${OpenCV_LIBS} Qt5::Core Qt5::Network Qt5::Xml Qt5::XmlPatterns Qt5::Script ${QUAZIP_LIBRARIES})
target_link_libraries( OpticalMapping BasicProcessing MatisseCore NavTools SystemTools ${OpenCV_LIBS} Qt5::Core Qt5::Network Qt5::Xml Qt5::XmlPatterns ${QUAZIP_LIBRARIES})


# state that anybody linking to us needs to include the current source dir
target_include_directories(OpticalMapping
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
