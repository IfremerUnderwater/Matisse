# add the executable
add_executable(MatisseGui
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
	src/RemoteJobHelper.cpp
    src/WelcomeDialog.cpp
    src/MainControllBar.cpp
    src/MatisseTabWidget.cpp
    src/AboutDialog.cpp
    src/RemoteFileDialog.cpp
    src/RemoteProgressDialog.cpp
    src/NetworkCheckDialog.cpp
    src/WheelGraphicsView.cpp
    BasicWidgets/MatisseMenu.cpp	
	BasicWidgets/MatisseVersionWidget.cpp
    BasicWidgets/HomeWidget.cpp
    BasicWidgets/VisuModeWidget.cpp
    BasicWidgets/MainMenuWidget.cpp	
    CameraUi/camera_manager_tool.cpp
    CameraUi/camera_calib_dialog.cpp
    ExpertUi/ExpertFormWidget.cpp
    ExpertUi/AssemblyDialog.cpp
    ExpertUi/AssemblyGraphicsScene.cpp
    ExpertUi/PipeWidget.cpp
    ExpertUi/ElementWidget.cpp
    ExpertUi/SourceWidget.cpp
    ExpertUi/ProcessorWidget.cpp
    ExpertUi/DestinationWidget.cpp
    ExpertUi/ElementWidgetProvider.cpp
    JobManagementUi/JobDialog.cpp
    JobManagementUi/DuplicateDialog.cpp
    JobManagementUi/RestoreJobsDialog.cpp
    ParametersUi/ParametersDialog.cpp
    ParametersUi/ParametersFoldButton.cpp
	RemoteJobUi/TreeItem.cpp
    RemoteJobUi/TreeModel.cpp
    UserUi/UserFormWidget.cpp
    UserUi/CartoScene.cpp
    UserUi/LiveProcessWheel.cpp
    UserUi/StatusMessageWidget.cpp
    UserUi/OngoingProcessWidget.cpp
    UserUi/PreferencesDialog.cpp
    #src/ParametersUi/EnrichedComboBox.cpp
    #src/ParametersUi/EnrichedFormWidget.cpp
    #src/ParametersUi/EnrichedLineEdit.cpp
    #src/ParametersUi/EnrichedSpinBox.cpp
    #src/ParametersUi/EnrichedDoubleSpinBox.cpp
    #src/ParametersUi/EnrichedCheckBox.cpp
    #src/ParametersUi/EnrichedListBox.cpp
    #src/ParametersUi/EnrichedTableWidget.cpp
	#src/ParametersUi/EnrichedCamComboBox.cpp
    #src/ParametersUi/ParametersWidgetSkeleton.cpp
    #src/ParametersUi/EnrichedFileChooser.cpp
	#src/ParametersUi/MatisseParametersManager.cpp
	#src/ParametersUi/EnrichedDecimalValueWidget.cpp
	#src/ParametersUi/ParametersHeaderButton.cpp
    #src/ParametersUi/FoldUnfoldButton.cpp	
    resources/resources.qrc
    style.qrc
    icons.qrc
    MatisseServer.rc)
	
	
set_target_properties(MatisseGui PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${MATISSE_RUNTIME_ROOT}/$<0:>)
	
include_directories(
    ${QUAZIP_INCLUDE_DIRS} 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/BasicWidgets
	${CMAKE_CURRENT_SOURCE_DIR}/CameraUi
    ${CMAKE_CURRENT_SOURCE_DIR}/ExpertUi
    ${CMAKE_CURRENT_SOURCE_DIR}/JobManagementUi
    ${CMAKE_CURRENT_SOURCE_DIR}/ParametersUi
    ${CMAKE_CURRENT_SOURCE_DIR}/RemoteJobUi
    ${CMAKE_CURRENT_SOURCE_DIR}/UserUi
    )

target_link_libraries(MatisseGui PUBLIC 
	 MatisseCore
     NetworkTools
     NavTools
     ${OpenCV_LIBS}
     Qt5::Core
     Qt5::Widgets
     #Qt5::Network
     Qt5::Xml
     Qt5::XmlPatterns
 #    Qt5::Script
     Qt5::OpenGL
     GDAL::GDAL
     #QSsh::QSsh
     #${BOTAN_LIB}
     #${QUAZIP_LIBRARIES}
     ${OPENSCENEGRAPH_LIBRARIES}
	 ${GeographicLib_LIBRARIES}
	 kmldom kmlbase)

# add the binary tree to the search path for include files
# so that we will find MatisseConfig.h

#target_include_directories(Matisse PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#						   ${EXTRA_INCLUDES}
#                           )
