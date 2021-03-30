# add the executable
add_library(MatisseCore
    src/LifecycleComponent.cpp	
    src/Processor.cpp
    src/ImageProvider.cpp
    src/RasterProvider.cpp
    src/MatisseEngine.cpp
    src/KeyValueList.cpp
    src/AssemblyDefinition.cpp
    src/JobDefinition.cpp
    src/JobServer.cpp
    src/MatissePreferences.cpp
    src/MatisseRemoteServerSettings.cpp
    src/SystemDataManager.cpp
    src/ProcessDataManager.cpp	
    src/Camera/camera_manager.cpp
    src/Camera/camera_info.cpp
    src/Camera/camera_calib.cpp
	src/GraphicalCharter/GraphicalCharter.cpp
	src/GraphicalCharter/MatisseIconFactory.cpp
    src/GraphicalCharter/IconizedWidgetWrapper.cpp
    src/GraphicalCharter/IconizedLabelWrapper.cpp
    src/GraphicalCharter/IconizedButtonWrapper.cpp
    src/GraphicalCharter/IconizedComboBoxItemWrapper.cpp
    src/GraphicalCharter/MatisseTreeItem.cpp
    src/GraphicalCharter/IconizedTreeItemWrapper.cpp
    src/GraphicalCharter/IconizedActionWrapper.cpp
    src/Parameters/MatisseDictionnaryLabels.h	
	src/Parameters/MatisseParameters.cpp
	# Temp location to resolve architectural issue
    src/Parameters/EnrichedComboBox.cpp
    src/Parameters/EnrichedFormWidget.cpp
    src/Parameters/EnrichedLineEdit.cpp
    src/Parameters/EnrichedSpinBox.cpp
    src/Parameters/EnrichedDoubleSpinBox.cpp
    src/Parameters/EnrichedCheckBox.cpp
    src/Parameters/EnrichedListBox.cpp
    src/Parameters/EnrichedTableWidget.cpp
	src/Parameters/EnrichedCamComboBox.cpp
    src/Parameters/ParametersWidgetSkeleton.cpp
    src/Parameters/EnrichedFileChooser.cpp
	src/Parameters/MatisseParametersManager.cpp
	src/Parameters/EnrichedDecimalValueWidget.cpp
	src/Parameters/ParametersHeaderButton.cpp
    src/Parameters/FoldUnfoldButton.cpp	
	
)
	
		
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR} 
	${CMAKE_CURRENT_SOURCE_DIR}/src 
	${CMAKE_CURRENT_SOURCE_DIR}/src/Camera
	${CMAKE_CURRENT_SOURCE_DIR}/src/GraphicalCharter
	${CMAKE_CURRENT_SOURCE_DIR}/src/Parameters
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
	${QUAZIP_LIBRARIES})

# add the binary tree to the search path for include files
# so that we will find MatisseConfig.h

#target_include_directories(Matisse PUBLIC
#                           "${PROJECT_BINARY_DIR}"
#						   ${EXTRA_INCLUDES}
#                           )

target_include_directories(MatisseCore
    INTERFACE 
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/Camera
	${CMAKE_CURRENT_SOURCE_DIR}/src/GraphicalCharter
	${CMAKE_CURRENT_SOURCE_DIR}/src/Parameters
		  )
