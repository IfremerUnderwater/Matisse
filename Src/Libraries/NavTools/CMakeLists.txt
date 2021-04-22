add_library(NavTools
	CartoImage.cpp
    gdal_translate_wrapper.cpp
    RasterGeoreferencer.cpp
    GeoTransform.cpp
    Dim2FileReader.cpp
    Dim2.cpp
)
	
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} NavTools_fr.ts NavTools_en.ts)

target_link_libraries(NavTools 
	MatisseImage 
	Qt5::Core 
	Qt5::Widgets 
	${OpenCV_LIBS} 
	GDAL::GDAL 
	${PROJ_LIBS}
	)


# state that anybody linking to us needs to include the current source dir
target_include_directories(NavTools
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
