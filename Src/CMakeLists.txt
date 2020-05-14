cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(MatisseProject VERSION 1.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4005")
set(PROJ_LIBS ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/proj.lib)
elseif(MSVC)
set(PROJ_LIBS "proj")
endif(MSVC)
# configure a header file to pass some of the CMake settings
# to the source code
configure_file(MatisseConfig.h.in MatisseConfig.h)

# qt5 also required for Quazip
find_package(Qt5 COMPONENTS Core Widgets Network Xml XmlPatterns Script OpenGL REQUIRED)

find_package( OpenCV REQUIRED )

find_package(QuaZip REQUIRED)

find_package(GDAL REQUIRED)

find_package(GeographicLib REQUIRED)

find_package(OpenSceneGraph REQUIRED osgDB osgUtil osgGA osgViewer)
# libOpenThreads & libosg automatically searched

find_package(OpenMVG REQUIRED)
find_package(cereal REQUIRED)
find_package(ceres REQUIRED)

# qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

add_subdirectory(Libraries)

add_subdirectory(MatisseServer)

add_subdirectory(Modules)

# add the executable
#add_executable(Matisse main.cxx)

#target_link_libraries(Matisse PUBLIC ${EXTRA_LIBS})

# add the binary tree to the search path for include files
# so that we will find MatisseConfig.h

#target_include_directories(Matisse PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#						   ${EXTRA_INCLUDES}
#                           )
