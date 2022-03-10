# add the executable
add_executable(gdal_translate
    commonutils.cpp
    gdal_translate_bin.cpp)
	
	
set_target_properties(gdal_translate PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)

if (UNIX)

    target_link_libraries(gdal_translate PUBLIC 
                    GDAL::GDAL
                    # ${GDAL_LIBRARY}
                    )

else (UNIX)

    target_link_libraries(gdal_translate PUBLIC 
                    ${GDAL_LIBRARY}
                    )
endif (UNIX)