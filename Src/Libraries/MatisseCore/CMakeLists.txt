# add the executable
add_library(MatisseCore
    LifecycleComponent.cpp	
    Processor.cpp
    ImageProvider.cpp
    RasterProvider.cpp
    MatisseEngine.cpp
    KeyValueList.cpp
    AssemblyDefinition.cpp
    JobDefinition.cpp
    JobServer.cpp
    MatissePreferences.cpp
    MatisseRemoteServerSettings.cpp
    SystemDataManager.cpp
    ProcessDataManager.cpp	
    Camera/camera_manager.cpp
    Camera/camera_info.cpp
    Camera/camera_calib.cpp
    Parameters/MatisseDictionnaryLabels.h	
	Parameters/MatisseParameters.cpp
	# Temp location to resolve architectural issue
    Parameters/EnrichedComboBox.cpp
    Parameters/EnrichedFormWidget.cpp
    Parameters/EnrichedLineEdit.cpp
    Parameters/EnrichedSpinBox.cpp
    Parameters/EnrichedDoubleSpinBox.cpp
    Parameters/EnrichedCheckBox.cpp
    Parameters/EnrichedListBox.cpp
    Parameters/EnrichedTableWidget.cpp
	Parameters/EnrichedCamComboBox.cpp
    Parameters/ParametersWidgetSkeleton.cpp
    Parameters/EnrichedFileChooser.cpp
	Parameters/MatisseParametersManager.cpp
	Parameters/EnrichedDecimalValueWidget.cpp
	Parameters/ParametersHeaderButton.cpp
    Parameters/FoldUnfoldButton.cpp	
	
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
