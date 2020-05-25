add_library(ImageProcessing imageprocessing.cpp)

target_link_libraries( ImageProcessing BasicProcessing ${OpenCV_LIBS} Qt5::Core)

# state that anybody linking to us needs to include the current source dir
target_include_directories(ImageProcessing
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
