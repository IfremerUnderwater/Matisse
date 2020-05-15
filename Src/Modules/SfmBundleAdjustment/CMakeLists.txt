add_library(SfmBundleAdjustment SHARED src/SfmBundleAdjustment.cpp)

target_link_libraries( SfmBundleAdjustment MatisseCommon BasicProcessing OpticalMapping ${OpenCV_LIBS} Qt5::Core Qt5::Xml Qt5::Widgets ${QUAZIP_LIBRARIES} GDAL::GDAL ${PROJ_LIBS})

set_target_properties(SfmBundleAdjustment PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)

add_custom_command(
 TARGET SfmBundleAdjustment POST_BUILD
 COMMAND ${CMAKE_COMMAND} -E copy ${MATISSE_RUNTIME_ROOT}/SfmBundleAdjustment.${D_EXT} ${MATISSE_RUNTIME_PROC_MODULES} 
 COMMENT "Copying plugin..."
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(SfmBundleAdjustment
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
