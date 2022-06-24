add_library(NetworkTools
    network_action.cpp
    network_action_file_transfer.cpp
    network_action_shell.cpp
    network_action_upload_file.cpp
    network_action_upload_dir.cpp
    network_action_download_dir.cpp
    network_action_dir_content.cpp
    network_action_send_command.cpp
    network_command.cpp
    network_command_pbs_qsub.cpp
    network_commons.cpp
    network_client.cpp
    network_client_file_transfer.cpp
    network_client_shell.cpp
    network_credentials.cpp
    network_connector.cpp
    network_connector_qftp.cpp
    network_connector_qssh.cpp
)
	
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} NetworkTools_fr.ts NetworkTools_en.ts)

include_directories(${QUAZIP_INCLUDE_DIRS})

target_link_libraries(NetworkTools
	SystemTools 
	Qt5::Core 
	Qt5::Network
	QSsh::QSsh 
	${BOTAN_LIB}
        QFtp
	)


# state that anybody linking to us needs to include the current source dir
target_include_directories(NetworkTools INTERFACE 
	${CMAKE_CURRENT_SOURCE_DIR}
    )
