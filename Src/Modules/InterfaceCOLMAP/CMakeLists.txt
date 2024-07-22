add_library(InterfaceColmap SHARED
    interface_colmap.cpp
    )

target_link_libraries(InterfaceColmap PRIVATE
    MatisseCore
    MatisseImage
    NavTools
    BasicProcessing
    OpticalMapping
    ${OpenCV_LIBS}
    Qt5::Core
    Qt5::Xml
    Qt5::Widgets
    ${QUAZIP_LIBRARIES}
    ${GDAL_LIBRARY}
    ${GeographicLib_LIBRARIES}
    OpenMVS::IO OpenMVS::MVS OpenMVS::Math OpenMVS::Common 
	CGAL::CGAL Boost::filesystem
	)

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(InterfaceColmap PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(InterfaceColmap PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(InterfaceColmap PROPERTIES PREFIX "")

add_custom_command(
 TARGET InterfaceColmap POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/InterfaceColmap.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(InterfaceColmap
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
