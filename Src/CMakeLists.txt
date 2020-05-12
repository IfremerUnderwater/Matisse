cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(MatisseProject VERSION 1.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file(MatisseConfig.h.in MatisseConfig.h)

# qt5 also required for Quazip
find_package(Qt5 COMPONENTS Core Widgets Network Xml REQUIRED)

# opencv
find_package( OpenCV REQUIRED )

# quazip
find_package(QuaZip REQUIRED)

# qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

add_subdirectory(Libraries)
list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/BasicProcessing")
list(APPEND EXTRA_LIBS BasicProcessing)

# add the executable
#add_executable(Matisse main.cxx)

#target_link_libraries(Matisse PUBLIC ${EXTRA_LIBS})

# add the binary tree to the search path for include files
# so that we will find MatisseConfig.h

#target_include_directories(Matisse PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#						   ${EXTRA_INCLUDES}
#                           )
