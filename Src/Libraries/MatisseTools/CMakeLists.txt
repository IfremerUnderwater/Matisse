add_library(MatisseTools
    src/httpImageDownloader.cpp
    src/TreeItem.cpp
    src/TreeModel.cpp
	)
	
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} MatisseTools_fr.ts MatisseTools_en.ts)

include_directories(${QUAZIP_INCLUDE_DIRS} MatisseCommon)

#target_link_libraries( MatisseTools MatisseCommon ${OpenCV_LIBS} Qt5::Core Qt5::Widgets Qt5::Network Qt5::Xml Qt5::XmlPatterns Qt5::Script ${QUAZIP_LIBRARIES})
target_link_libraries( MatisseTools MatisseCommon ${OpenCV_LIBS} Qt5::Core Qt5::Widgets Qt5::Network Qt5::Xml Qt5::XmlPatterns ${QUAZIP_LIBRARIES})



# state that anybody linking to us needs to include the current source dir
target_include_directories(MatisseTools
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
