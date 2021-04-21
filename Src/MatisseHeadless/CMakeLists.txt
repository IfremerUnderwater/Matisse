# add the executable
add_executable(MatisseHeadless
    main.cpp
    JobLauncher.cpp
    CartoImage.cpp)
	
	
set_target_properties(MatisseHeadless PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
	
include_directories(${QUAZIP_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(MatisseHeadless PUBLIC 
	MatisseCore NavTools 
	${OpenCV_LIBS} 
	Qt5::Core 
	Qt5::Widgets 
	Qt5::Network 
	Qt5::Xml 
	Qt5::XmlPatterns 
	Qt5::OpenGL 
	GDAL::GDAL 
	${QUAZIP_LIBRARIES} 
	${OPENSCENEGRAPH_LIBRARIES}
	)

# add the binary tree to the search path for include files
# so that we will find MatisseConfig.h

#target_include_directories(Matisse PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#						   ${EXTRA_INCLUDES}
#                           )
