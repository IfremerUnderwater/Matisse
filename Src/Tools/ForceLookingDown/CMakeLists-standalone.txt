cmake_minimum_required(VERSION 3.0)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(force_looking_down)

#--------------------------------------------------------------------
# Hide the console window in visual studio projects - Release
#--------------------------------------------------------------------
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()

add_definitions(-std=c++11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(OpenMVG REQUIRED)
find_package(cereal REQUIRED)
find_package(ceres REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(force_looking_down force_looking_down_gui.ui force_looking_down_gui.cpp main.cpp)
target_link_libraries(force_looking_down
  PRIVATE
    OpenMVG::openMVG_camera
    OpenMVG::openMVG_exif
    OpenMVG::openMVG_features
	OpenMVG::openMVG_stlplus
    OpenMVG::openMVG_geodesy
    OpenMVG::openMVG_geometry
    OpenMVG::openMVG_graph
    OpenMVG::openMVG_image
    OpenMVG::openMVG_linearProgramming
    OpenMVG::openMVG_matching
    OpenMVG::openMVG_matching_image_collection
    OpenMVG::openMVG_multiview
    OpenMVG::openMVG_numeric
    OpenMVG::openMVG_robust_estimation
    OpenMVG::openMVG_sfm
    OpenMVG::openMVG_system
	Qt5::Widgets
)

