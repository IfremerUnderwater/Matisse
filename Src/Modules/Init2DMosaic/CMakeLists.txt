add_library(Init2DMosaic SHARED src/Init2DMosaic.cpp)

set_target_properties(Init2DMosaic PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)

target_link_libraries( Init2DMosaic MatisseCommon BasicProcessing OpticalMapping ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES} GDAL::GDAL ${PROJ_LIBS})

add_custom_command(
 TARGET Init2DMosaic POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/Init2DMosaic.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(Init2DMosaic
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )