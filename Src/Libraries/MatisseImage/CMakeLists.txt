add_library(MatisseImage
    image.cpp
    image_listener.cpp
    image_set.cpp
    file_image.cpp
    nav_image.cpp
    nav_info.cpp
    picture_file_set.cpp
)
	
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} NavTools_fr.ts NavTools_en.ts)


#target_link_libraries( MatisseTools MatisseCommon ${OpenCV_LIBS} Qt5::Core Qt5::Widgets Qt5::Network Qt5::Xml Qt5::XmlPatterns Qt5::Script ${QUAZIP_LIBRARIES})
target_link_libraries( MatisseImage Qt5::Core Qt5::Widgets ${OpenCV_LIBS} ${GDAL_LIBRARY} ${PROJ_LIBS})



# state that anybody linking to us needs to include the current source dir
target_include_directories(MatisseImage
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
