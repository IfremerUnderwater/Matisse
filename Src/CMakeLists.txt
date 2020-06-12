cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(MatisseProject VERSION 1.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4005")
set(PROJ_LIBS ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/proj.lib)
set(OPENMVG_INSTALL_PATH ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})
set(D_EXT "dll")
else(MSVC)
set(PROJ_LIBS "proj")
set(D_EXT "so")
endif(MSVC)

# define Runtime directories as config dependent folders (and create them)
set(MATISSE_RUNTIME_ROOT ${PROJECT_BINARY_DIR}/Run/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>)
set(MATISSE_RUNTIME_PROC_MODULES ${MATISSE_RUNTIME_ROOT}/Modules/processors)
set(MATISSE_RUNTIME_IN_MODULES ${MATISSE_RUNTIME_ROOT}/Modules/imageProviders)
set(MATISSE_RUNTIME_OUT_MODULES ${MATISSE_RUNTIME_ROOT}/Modules/rasterProviders)

add_custom_target(create_proc_folder ALL
 COMMAND ${CMAKE_COMMAND} -E make_directory ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Creating processors modules folder ..."
)
add_custom_target(create_in_mod_folder ALL
 COMMAND ${CMAKE_COMMAND} -E make_directory ${MATISSE_RUNTIME_IN_MODULES} 
 COMMENT "Creating input modules folder ..."
)
add_custom_target(create_out_mod_folder ALL
 COMMAND ${CMAKE_COMMAND} -E make_directory ${MATISSE_RUNTIME_OUT_MODULES} 
 COMMENT "Creating output modules folder ..."
)

add_custom_target(copy_config ALL
 COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/../Config ${MATISSE_RUNTIME_ROOT} 
 COMMENT "Copying config folder ..."
)


add_custom_target(copy_install ALL
 COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/../Install ${PROJECT_BINARY_DIR}/Install
 COMMENT "Copying install folder ..."
)

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
find_package(cereal)
find_package(ceres REQUIRED)

find_package(CGAL)
FIND_PACKAGE(Boost COMPONENTS iostreams program_options filesystem system serialization)
find_package(OpenMVS REQUIRED)

find_package(LibKML REQUIRED)

# qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

add_subdirectory(Libraries)

add_subdirectory(MatisseServer)
add_subdirectory(MatisseCmd)

add_subdirectory(Modules)

add_subdirectory(Tools)
