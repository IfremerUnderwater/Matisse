# add the executable
add_library(MatisseCore
    assembly_definition.cpp
    job_definition.cpp
    job_server.cpp
    key_value_list.cpp
    lifecycle_component.cpp
    matisse_engine.cpp
    matisse_preferences.cpp
    matisse_remote_server_settings.cpp
    process_data_manager.cpp
    processor.cpp
    input_data_provider.cpp
    output_data_writer.cpp
    system_data_manager.cpp
    Camera/camera_manager.cpp
    Camera/camera_info.cpp
    Camera/camera_calib.cpp
    Parameters/MatisseDictionnaryLabels.h	
    Parameters/matisse_parameters.cpp
    # Temp location to resolve architectural issue
    GraphicalCharter/graphical_charter.cpp
    GraphicalCharter/iconized_action_wrapper.cpp
    GraphicalCharter/iconized_button_wrapper.cpp
    GraphicalCharter/iconized_combo_box_item_wrapper.cpp
    GraphicalCharter/iconized_label_wrapper.cpp
    GraphicalCharter/iconized_tree_item_wrapper.cpp
    GraphicalCharter/iconized_widget_wrapper.cpp
    GraphicalCharter/matisse_icon_factory.cpp
    GraphicalCharter/matisse_tree_item.cpp
    Parameters/enriched_cam_combo_box.cpp
    Parameters/enriched_check_box.cpp
    Parameters/enriched_combo_box.cpp
    Parameters/enriched_decimal_value_widget.cpp
    Parameters/enriched_double_spin_box.cpp
    Parameters/enriched_file_chooser.cpp
    Parameters/enriched_form_widget.cpp
    Parameters/enriched_line_edit.cpp
    Parameters/enriched_list_box.cpp
    Parameters/enriched_spin_box.cpp
    Parameters/enriched_table_widget.cpp
    Parameters/fold_unfold_button.cpp
    Parameters/matisse_parameters_manager.cpp
    Parameters/parameters_header_button.cpp
    Parameters/parameters_widget_skeleton.cpp
)
	
		
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR} 
	${CMAKE_CURRENT_SOURCE_DIR}/Camera
	${CMAKE_CURRENT_SOURCE_DIR}/GraphicalCharter
	${CMAKE_CURRENT_SOURCE_DIR}/Parameters
	)

#target_link_libraries(MatisseCmd PUBLIC MatisseTools MatisseCommon ${OpenCV_LIBS} Qt5::Core Qt5::Widgets Qt5::Network Qt5::Xml Qt5::XmlPatterns Qt5::Script Qt5::OpenGL GDAL::GDAL ${QUAZIP_LIBRARIES} ${OPENSCENEGRAPH_LIBRARIES})
#target_link_libraries(MatisseCmd PUBLIC MatisseTools MatisseCommon ${OpenCV_LIBS} Qt5::Core Qt5::Widgets Qt5::Network Qt5::Xml Qt5::XmlPatterns Qt5::OpenGL GDAL::GDAL ${QUAZIP_LIBRARIES} ${OPENSCENEGRAPH_LIBRARIES})
target_link_libraries(MatisseCore PUBLIC 
	MatisseImage 
	SystemTools 
	BasicProcessing
	Qt5::Core 
	Qt5::Widgets
	Qt5::Xml
	Qt5::XmlPatterns	
	Qt5::Network 
	${QUAZIP_LIBRARIES}
	)

# add the binary tree to the search path for include files
# so that we will find MatisseConfig.h

#target_include_directories(Matisse PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#						   ${EXTRA_INCLUDES}
#                           )

target_include_directories(MatisseCore
    INTERFACE 
	${CMAKE_CURRENT_SOURCE_DIR} 
	${CMAKE_CURRENT_SOURCE_DIR}/Camera
	${CMAKE_CURRENT_SOURCE_DIR}/GraphicalCharter
	${CMAKE_CURRENT_SOURCE_DIR}/Parameters
	)
