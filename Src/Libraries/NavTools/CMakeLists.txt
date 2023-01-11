add_library(NavTools	
    carto_image.cpp
    dim2.cpp
    dim2_file_reader.cpp
    gdal_translate_wrapper.cpp
    geo_transform.cpp
    raster_georeferencer.cpp
)
	
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} NavTools_fr.ts NavTools_en.ts)

target_link_libraries(NavTools 
	MatisseImage 
	Qt5::Core 
	Qt5::Widgets 
	${OpenCV_LIBS} 
	${GDAL_LIBRARY} 
	${PROJ_LIBS}
	)


# state that anybody linking to us needs to include the current source dir
target_include_directories(NavTools
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    )
