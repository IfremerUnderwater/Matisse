add_library(BridgeModule SHARED src/BridgeModule.cpp)

target_link_libraries( BridgeModule MatisseCommon BasicProcessing OpticalMapping ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES})

set_target_properties(BridgeModule PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)

add_custom_command(
 TARGET BridgeModule POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/BridgeModule.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(BridgeModule
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
