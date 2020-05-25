add_library(ReconstructionLister SHARED src/ReconstructionLister.cpp)

set_target_properties(ReconstructionLister PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)

target_link_libraries( ReconstructionLister MatisseCommon BasicProcessing ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES} GDAL::GDAL ${PROJ_LIBS})

add_custom_command(
 TARGET ReconstructionLister POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/ReconstructionLister.${D_EXT} ${MATISSE_RUNTIME_OUT_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(ReconstructionLister
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
