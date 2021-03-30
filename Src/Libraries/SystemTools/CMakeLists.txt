add_library(SystemTools
    src/FileUtils.cpp
    src/StringUtils.cpp
    src/PlatformComponent.cpp
    src/PlatformInspector.cpp
    src/PlatformDump.cpp
    src/PlatformOpencvComponent.cpp
    src/platformOsgComponent.cpp
    src/PlatformComparisonStatus.cpp
    src/PlatformComparator.cpp
)
	
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} SystemTools_fr.ts SystemTools_en.ts)

include_directories(${QUAZIP_INCLUDE_DIRS})

#target_link_libraries( MatisseTools MatisseCommon ${OpenCV_LIBS} Qt5::Core Qt5::Widgets Qt5::Network Qt5::Xml Qt5::XmlPatterns Qt5::Script ${QUAZIP_LIBRARIES})
target_link_libraries( 
SystemTools 
Qt5::Core 
Qt5::Widgets 
${QUAZIP_LIBRARIES}
)



# state that anybody linking to us needs to include the current source dir
target_include_directories(SystemTools
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} 
		  ${CMAKE_CURRENT_SOURCE_DIR}/src 
		  ${QUAZIP_INCLUDE_DIRS}
		  #C:/vcpkg/installed/x64-windows/include/Qt5Quazip
          )
