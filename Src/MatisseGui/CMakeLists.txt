# add the executable
add_executable(MatisseGui
    main.cpp
    about_dialog.cpp
    main_gui.cpp
    welcome_dialog.cpp
    BasicWidgets/home_widget.cpp
    BasicWidgets/main_controll_bar.cpp
    BasicWidgets/main_menu_widget.cpp
    BasicWidgets/matisse_menu.cpp
    BasicWidgets/matisse_tab_widget.cpp
    BasicWidgets/matisse_version_widget.cpp
    BasicWidgets/visu_mode_widget.cpp
    CameraUi/camera_manager_tool.cpp
    CameraUi/camera_calib_dialog.cpp
    ExpertUi/assembly_editor.cpp
    ExpertUi/assembly_dialog.cpp
    ExpertUi/assembly_graphics_scene.cpp
    ExpertUi/destination_widget.cpp
    ExpertUi/element_widget.cpp
    ExpertUi/element_widget_provider.cpp
    ExpertUi/pipe_widget.cpp
    ExpertUi/processor_widget.cpp
    ExpertUi/source_widget.cpp
    JobManagementUi/duplicate_dialog.cpp
    JobManagementUi/job_dialog.cpp
    #JobManagementUi/job_file_helper.cpp
    JobManagementUi/restore_jobs_dialog.cpp
    ParametersUi/parameters_dialog.cpp
    ParametersUi/parameters_fold_button.cpp
    RemoteJobUi/remote_file_dialog.cpp
    RemoteJobUi/remote_job_helper.cpp
    RemoteJobUi/remote_progress_dialog.cpp
    RemoteJobUi/tree_item.cpp
    RemoteJobUi/tree_model.cpp
    UserUi/data_viewer.cpp
    UserUi/carto_scene.cpp
    UserUi/live_process_wheel.cpp
    UserUi/ongoing_process_widget.cpp
    UserUi/preferences_dialog.cpp
    UserUi/status_message_widget.cpp
    UserUi/wheel_graphics_view.cpp
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
