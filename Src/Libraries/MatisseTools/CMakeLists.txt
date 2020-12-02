add_library(MatisseTools
    src/MatisseDictionnaryLabels.h
    src/ParametersWidgetSkeleton.cpp
    src/KeyValueList.cpp
    src/AssemblyDefinition.cpp
    src/EnrichedComboBox.cpp
    src/EnrichedFormWidget.cpp
    src/EnrichedLineEdit.cpp
    src/EnrichedSpinBox.cpp
    src/EnrichedDoubleSpinBox.cpp
    src/EnrichedCheckBox.cpp
    src/EnrichedListBox.cpp
    src/EnrichedTableWidget.cpp
    src/JobDefinition.cpp
    src/JobServer.cpp
    src/EnrichedFileChooser.cpp
    src/MatissePreferences.cpp
    src/MatisseParametersManager.cpp
    src/EnrichedDecimalValueWidget.cpp
    src/httpImageDownloader.cpp
    src/ParametersHeaderButton.cpp
    src/FoldUnfoldButton.cpp
    src/SystemDataManager.cpp
    src/ProcessDataManager.cpp
    src/PlatformComponent.cpp
    src/PlatformInspector.cpp
    src/PlatformDump.cpp
    src/PlatformOpencvComponent.cpp
    src/platformOsgComponent.cpp
    src/PlatformComparisonStatus.cpp
    src/PlatformComparator.cpp
    src/MatisseIconFactory.cpp
    src/IconizedWidgetWrapper.cpp
    src/IconizedLabelWrapper.cpp
    src/IconizedButtonWrapper.cpp
    src/IconizedComboBoxItemWrapper.cpp
    src/MatisseTreeItem.cpp
    src/IconizedTreeItemWrapper.cpp
    src/IconizedActionWrapper.cpp
    src/EnrichedCamComboBox.cpp)
	
#qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} MatisseTools_fr.ts MatisseTools_en.ts)

include_directories(${QUAZIP_INCLUDE_DIRS} MatisseCommon)

target_link_libraries( MatisseTools MatisseCommon ${OpenCV_LIBS} Qt5::Core Qt5::Widgets Qt5::Network Qt5::Xml Qt5::XmlPatterns Qt5::Script ${QUAZIP_LIBRARIES})

# state that anybody linking to us needs to include the current source dir
target_include_directories(MatisseTools
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src
          )
