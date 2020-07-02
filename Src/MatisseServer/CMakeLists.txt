# add the executable
add_executable(MatisseServer
    src/main.cpp
    src/SshClient.cpp
    src/SshAction.cpp
    src/AssemblyGui.cpp
    src/QSshClient.cpp
    src/ProcessorWidget.cpp
	src/RemoteJobManager.cpp
    src/SourceWidget.cpp
    src/ElementWidget.cpp
    src/DestinationWidget.cpp
    src/AssemblyGraphicsScene.cpp
    src/PipeWidget.cpp
    src/AssemblyDialog.cpp
    src/ExpertFormWidget.cpp
	src/SshClientStub.cpp
    src/UserFormWidget.cpp
    src/ParametersDialog.cpp
    src/Server.cpp
    src/JobDialog.cpp
    src/StatusMessageWidget.cpp
    src/WelcomeDialog.cpp
    src/HomeWidget.cpp
    src/VisuModeWidget.cpp
    src/MainMenuWidget.cpp
    src/OngoingProcessWidget.cpp
    src/MainControllBar.cpp
    src/MatisseVersionWidget.cpp
    src/MatisseTabWidget.cpp
    src/MatisseMenu.cpp
    src/PreferencesDialog.cpp
    src/LiveProcessWheel.cpp
    src/ParametersFoldButton.cpp
    src/ElementWidgetProvider.cpp
    src/AboutDialog.cpp
    src/OSGWidget.cpp
    src/DuplicateDialog.cpp
    src/RestoreJobsDialog.cpp
    src/NetworkCheckDialog.cpp
    src/CartoScene.cpp
    src/CartoImage.cpp
    src/WheelGraphicsView.cpp
	resources/resources.qrc
    style.qrc
    icons.qrc)
	
	
set_target_properties(MatisseServer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
	
include_directories(${QUAZIP_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(MatisseServer PUBLIC 
     MatisseTools
     MatisseCommon
     ${OpenCV_LIBS}
     Qt5::Core
     Qt5::Widgets
     Qt5::Network
     Qt5::Xml
     Qt5::XmlPatterns
 #    Qt5::Script
     Qt5::OpenGL
     GDAL::GDAL
     QSsh::QSsh
     ${BOTAN_LIB}
     ${QUAZIP_LIBRARIES}
     ${OPENSCENEGRAPH_LIBRARIES})

# add the binary tree to the search path for include files
# so that we will find MatisseConfig.h

#target_include_directories(Matisse PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#						   ${EXTRA_INCLUDES}
#                           )
