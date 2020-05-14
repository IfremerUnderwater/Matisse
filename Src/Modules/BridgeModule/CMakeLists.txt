add_library(BridgeModule SHARED src/BridgeModule.cpp)

target_link_libraries( BridgeModule MatisseCommon BasicProcessing OpticalMapping ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES})

# state that anybody linking to us needs to include the current source dir
target_include_directories(BridgeModule
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
