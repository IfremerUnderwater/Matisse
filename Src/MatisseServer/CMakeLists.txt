# add the executable
add_executable(MatisseServer
    src/main.cpp
	src/OSGWidget/kml_handler.cpp
	src/OSGWidget/point3d.cpp
	src/OSGWidget/osg_widget_tool.cpp
    src/OSGWidget/osg_widget.cpp
    src/OSGWidget/box_visitor.cpp
    src/OSGWidget/area_computation_visitor.cpp
    src/OSGWidget/measurement_picker_tool.cpp
    src/OSGWidget/minmax_computation_visitor.cpp
    src/OSGWidget/geometry_type_count_visitor.cpp
    src/OSGWidget/shader_color.cpp
    src/OSGWidget/overlay_widget.cpp
    src/AssemblyGui.cpp
    src/ProcessorWidget.cpp
	src/RemoteJobHelper.cpp
    src/SourceWidget.cpp
    src/ElementWidget.cpp
    src/DestinationWidget.cpp
    src/AssemblyGraphicsScene.cpp
    src/PipeWidget.cpp
    src/AssemblyDialog.cpp
    src/ExpertFormWidget.cpp
    src/UserFormWidget.cpp
    src/ParametersDialog.cpp
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
    src/DuplicateDialog.cpp
    src/RemoteFileDialog.cpp
    src/RemoteProgressDialog.cpp
    src/RestoreJobsDialog.cpp
    src/NetworkCheckDialog.cpp
    src/CartoScene.cpp
    src/CartoImage.cpp
    src/WheelGraphicsView.cpp
	resources/resources.qrc
    style.qrc
    icons.qrc
	MatisseServer.rc)
	
	
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
     ${OPENSCENEGRAPH_LIBRARIES}
	 ${GeographicLib_LIBRARIES}
	 kmldom kmlbase)

# add the binary tree to the search path for include files
# so that we will find MatisseConfig.h

#target_include_directories(Matisse PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#						   ${EXTRA_INCLUDES}
#                           )
