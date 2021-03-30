add_library(DrawBlend2DMosaic SHARED src/DrawBlend2DMosaic.cpp)

# linux treat it as a lib and windows as a binary so we set both prop and remove lib prefix
set_target_properties(DrawBlend2DMosaic PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(DrawBlend2DMosaic PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
set_target_properties(DrawBlend2DMosaic PROPERTIES PREFIX "")

include_directories(${QUAZIP_INCLUDE_DIRS})

target_link_libraries( DrawBlend2DMosaic MatisseCore BasicProcessing OpticalMapping ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES} GDAL::GDAL ${PROJ_LIBS})

add_custom_command(
 TARGET DrawBlend2DMosaic POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/DrawBlend2DMosaic.${D_EXT} ${MATISSE_RUNTIME_OUT_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(DrawBlend2DMosaic
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
