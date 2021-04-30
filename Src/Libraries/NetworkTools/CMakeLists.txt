add_library(NetworkTools
    network_action.cpp
    SshCommand.cpp
    SshCommandPbsQsub.cpp
    network_client.cpp
    QSshClient.cpp
)
	
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} NetworkTools_fr.ts NetworkTools_en.ts)

include_directories(${QUAZIP_INCLUDE_DIRS})

target_link_libraries(NetworkTools
	SystemTools 
	Qt5::Core 
	Qt5::Network
	QSsh::QSsh 
	${BOTAN_LIB}
	)


# state that anybody linking to us needs to include the current source dir
target_include_directories(NetworkTools INTERFACE 
	${CMAKE_CURRENT_SOURCE_DIR}
    )
