add_library(NetworkTools
    network_action.cpp
    network_file_action.cpp
    network_command_action.cpp
    network_action_upload_file.cpp
    network_action_upload_dir.cpp
    network_action_download_dir.cpp
    network_action_dir_content.cpp
    network_action_send_command.cpp
    network_command.cpp
    network_command_pbs_qsub.cpp
    network_client.cpp
    network_file_client.cpp
    network_command_client.cpp
    sftp_client.cpp
    ssh_client.cpp
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
