add_library(SystemTools
    file_utils.cpp
    string_utils.cpp
    platform_comparator.cpp
    platform_comparison_status.cpp
    platform_component.cpp
    platform_dump.cpp
    platform_inspector.cpp
    platform_opencv_component.cpp
    platform_osg_component.cpp
)
	
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} SystemTools_fr.ts SystemTools_en.ts)

include_directories(
	${QUAZIP_INCLUDE_DIRS}
	)

target_link_libraries( 
	SystemTools 
	Qt5::Core 
	Qt5::Widgets 
        ${OpenCV_LIBS}
	${QUAZIP_LIBRARIES}
        ${OPENSCENEGRAPH_LIBRARIES}
	)

# state that anybody linking to us needs to include the current source dir
target_include_directories(SystemTools INTERFACE 
	${CMAKE_CURRENT_SOURCE_DIR} 
	${QUAZIP_INCLUDE_DIRS}
    )
