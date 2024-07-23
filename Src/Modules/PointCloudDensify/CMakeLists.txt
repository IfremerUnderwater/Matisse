add_library(PointCloudDensify SHARED
    point_cloud_densify.cpp
    )

target_link_libraries(PointCloudDensify
	MatisseCore
	MatisseImage
	BasicProcessing
	${OpenCV_LIBS}
	Qt5::Core
	Qt5::Xml
	Qt5::Widgets
	${QUAZIP_LIBRARIES}
	${GDAL_LIBRARY}
	OpenMVS::IO OpenMVS::MVS OpenMVS::Math OpenMVS::Common 
	CGAL::CGAL Boost::filesystem
	)

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(PointCloudDensify PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(PointCloudDensify PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(PointCloudDensify PROPERTIES PREFIX "")

add_custom_command(
 TARGET PointCloudDensify POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/PointCloudDensify.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(PointCloudDensify
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
