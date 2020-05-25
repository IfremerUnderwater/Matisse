add_library(Flusher SHARED src/Flusher.cpp)

set_target_properties(Flusher PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)

target_link_libraries( Flusher MatisseCommon BasicProcessing ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES} GDAL::GDAL ${PROJ_LIBS})

add_custom_command(
 TARGET Flusher POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/Flusher.${D_EXT} ${MATISSE_RUNTIME_IN_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(Flusher
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
