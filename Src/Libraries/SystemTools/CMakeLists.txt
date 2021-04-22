add_library(SystemTools
    FileUtils.cpp
    StringUtils.cpp
    PlatformComponent.cpp
    PlatformInspector.cpp
    PlatformDump.cpp
    PlatformOpencvComponent.cpp
    platformOsgComponent.cpp
    PlatformComparisonStatus.cpp
    PlatformComparator.cpp
)
	
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} SystemTools_fr.ts SystemTools_en.ts)

include_directories(
	${QUAZIP_INCLUDE_DIRS}
	)

target_link_libraries( 
	SystemTools 
	Qt5::Core 
	Qt5::Widgets 
	${QUAZIP_LIBRARIES}
	)

# state that anybody linking to us needs to include the current source dir
target_include_directories(SystemTools INTERFACE 
	${CMAKE_CURRENT_SOURCE_DIR} 
	${QUAZIP_INCLUDE_DIRS}
    )
