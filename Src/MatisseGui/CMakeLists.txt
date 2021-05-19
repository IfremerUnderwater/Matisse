# add the executable
add_executable(MatisseGui
    main.cpp
    AssemblyGui.cpp
    WelcomeDialog.cpp
    AboutDialog.cpp
    BasicWidgets/MainControllBar.cpp
    BasicWidgets/MainMenuWidget.cpp	
    BasicWidgets/MatisseMenu.cpp	
	BasicWidgets/MatisseVersionWidget.cpp
    BasicWidgets/HomeWidget.cpp
    BasicWidgets/VisuModeWidget.cpp
    BasicWidgets/MatisseTabWidget.cpp
    CameraUi/camera_manager_tool.cpp
    CameraUi/camera_calib_dialog.cpp
    ExpertUi/assembly_editor.cpp
    ExpertUi/AssemblyDialog.cpp
    ExpertUi/AssemblyGraphicsScene.cpp
    ExpertUi/PipeWidget.cpp
    ExpertUi/ElementWidget.cpp
    ExpertUi/SourceWidget.cpp
    ExpertUi/ProcessorWidget.cpp
    ExpertUi/DestinationWidget.cpp
    ExpertUi/ElementWidgetProvider.cpp
    JobManagementUi/job_file_helper.cpp
    JobManagementUi/JobDialog.cpp
    JobManagementUi/DuplicateDialog.cpp
    JobManagementUi/RestoreJobsDialog.cpp
    ParametersUi/ParametersDialog.cpp
    ParametersUi/ParametersFoldButton.cpp
#    RealTimeUi/NetworkCheckDialog.cpp
    RemoteJobUi/RemoteJobHelper.cpp
    RemoteJobUi/RemoteFileDialog.cpp
    RemoteJobUi/RemoteProgressDialog.cpp
    RemoteJobUi/TreeItem.cpp
    RemoteJobUi/TreeModel.cpp
    UserUi/data_viewer.cpp
    UserUi/CartoScene.cpp
    UserUi/WheelGraphicsView.cpp
    UserUi/LiveProcessWheel.cpp
    UserUi/StatusMessageWidget.cpp
    UserUi/OngoingProcessWidget.cpp
    UserUi/PreferencesDialog.cpp
    UserUi/OSG3DViewer/kml_handler.cpp
    UserUi/OSG3DViewer/point3d.cpp
    UserUi/OSG3DViewer/osg_widget_tool.cpp
    UserUi/OSG3DViewer/osg_widget.cpp
    UserUi/OSG3DViewer/box_visitor.cpp
    UserUi/OSG3DViewer/area_computation_visitor.cpp
    UserUi/OSG3DViewer/measurement_picker_tool.cpp
    UserUi/OSG3DViewer/minmax_computation_visitor.cpp
    UserUi/OSG3DViewer/geometry_type_count_visitor.cpp
    UserUi/OSG3DViewer/shader_color.cpp
    UserUi/OSG3DViewer/overlay_widget.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/BasicWidgets
	${CMAKE_CURRENT_SOURCE_DIR}/CameraUi
    ${CMAKE_CURRENT_SOURCE_DIR}/ExpertUi
    ${CMAKE_CURRENT_SOURCE_DIR}/JobManagementUi
    ${CMAKE_CURRENT_SOURCE_DIR}/ParametersUi
    ${CMAKE_CURRENT_SOURCE_DIR}/RemoteJobUi
    ${CMAKE_CURRENT_SOURCE_DIR}/RealTimeUi
    ${CMAKE_CURRENT_SOURCE_DIR}/UserUi
    ${CMAKE_CURRENT_SOURCE_DIR}/UserUi/OSG3DViewer
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
