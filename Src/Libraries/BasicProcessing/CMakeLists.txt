add_library(BasicProcessing Polygon.cpp gpc.cpp stdvectoperations.cpp)

target_link_libraries( BasicProcessing Qt5::Core)

# state that anybody linking to us needs to include the current source dir
target_include_directories(BasicProcessing
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
