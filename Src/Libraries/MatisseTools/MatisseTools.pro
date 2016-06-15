#-------------------------------------------------
#
# Project created by QtCreator 2013-12-09T17:59:55
#
#-------------------------------------------------

TARGET = MatisseTools
TEMPLATE = lib
CONFIG += staticlib

QT       += script xml xmlpatterns network gui

win32 {
    QMAKE_CXXFLAGS += /wd4100 /wd4996
}

# Workaround to be removed in qt5 with qmake.conf and shadowed function --
SOURCE_DIR=$$PWD/../../
CONFIG(debug, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Debug
}
CONFIG(release, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Release
}
# ------------------------------------------------------------------------

include(../../Scripts/MatisseCommon.pri)
include(../../Scripts/opencv.pri)
include(../../Scripts/qgis.pri)
include(../../Scripts/ProtoBuf.pri)
include(../../Scripts/QuaZIP.pri)

win32{
    CONFIG(release, debug|release) {
        include(../../Scripts/OpenSceneGraph.pri)
        DEFINES *= WITH_OSG
    }
}
unix{
    include(../../Scripts/OpenSceneGraph.pri)
    DEFINES *= WITH_OSG
}


INCLUDEPATH += src

SOURCES +=\
    src/ParametersWidgetSkeleton.cpp \
    src/KeyValueList.cpp \
    src/AssemblyDefinition.cpp \
    src/Xml.cpp \
    src/EnrichedComboBox.cpp \
    src/EnrichedFormWidget.cpp \
    src/EnrichedLineEdit.cpp \
    src/EnrichedSpinBox.cpp \
    src/EnrichedDoubleSpinBox.cpp \
    src/EnrichedCheckBox.cpp \
    src/EnrichedListBox.cpp \
    src/EnrichedTableWidget.cpp \
    src/JobDefinition.cpp \
    src/JobServer.cpp \
    src/EnrichedFileChooser.cpp \
    src/MatissePreferences.cpp \
    src/MatisseParametersManager.cpp \
    src/EnrichedDecimalValueWidget.cpp \
    src/httpImageDownloader.cpp \
    src/ParametersHeaderButton.cpp \
    src/FoldUnfoldButton.cpp \
    src/SystemDataManager.cpp \
    src/ProcessDataManager.cpp \
    src/PlatformComponent.cpp \
    src/PlatformInspector.cpp \
    src/PlatformDump.cpp \
    src/PlatformQgisComponent.cpp \
    src/PlatformOpencvComponent.cpp \
    src/platformOsgComponent.cpp \
    src/PlatformComparisonStatus.cpp \
    src/PlatformComparator.cpp \
    src/MatisseIconFactory.cpp \
    src/IconizedWidgetWrapper.cpp \
    src/IconizedLabelWrapper.cpp \
    src/IconizedButtonWrapper.cpp \
    src/IconizedComboBoxItemWrapper.cpp \
    src/MatisseTreeItem.cpp \
    src/IconizedTreeItemWrapper.cpp \
    src/IconizedActionWrapper.cpp

HEADERS +=\
    src/ParametersWidgetSkeleton.h \
    src/KeyValueList.h \
    src/AssemblyDefinition.h \
    src/Xml.h \
    src/EnrichedComboBox.h \
    src/EnrichedFormWidget.h \
    src/EnrichedLineEdit.h \
    src/EnrichedSpinBox.h \
    src/EnrichedDoubleSpinBox.h \
    src/EnrichedCheckBox.h \
    src/EnrichedListBox.h \
    src/EnrichedTableWidget.h \
    src/JobDefinition.h \
    src/JobServer.h \
    src/EnrichedFileChooser.h \
    src/ToolsCommon.h \
    src/MatissePreferences.h \
    src/MatisseParametersManager.h \
    src/MatisseDictionnaryLabels.h \
    src/EnrichedDecimalValueWidget.h \
    src/httpImageDownloader.h \
    src/ParametersHeaderButton.h \
    src/FoldUnfoldButton.h \
    src/SystemDataManager.h \
    src/ProcessDataManager.h \
    src/PlatformComponent.h \
    src/PlatformInspector.h \
    src/PlatformDump.h \
    src/PlatformQgisComponent.h \
    src/PlatformOpencvComponent.h \
    src/PlatformOsgComponent.h \
    src/PlatformComparisonStatus.h \
    src/PlatformComparator.h \
    src/MatisseIconFactory.h \
    src/IconizedWidgetWrapper.h \
    src/IconizedLabelWrapper.h \
    src/IconizedButtonWrapper.h \
    src/IconizedComboBoxItemWrapper.h \
    src/MatisseTreeItem.h \
    src/IconizedTreeItemWrapper.h \
    src/IconizedActionWrapper.h

FORMS    += \
            ui/ParametersWidgetSkeleton.ui

TRANSLATIONS=MatisseTools_fr.ts \
        MatisseTools_en.ts

DISTFILES += \
    ../../../Config/schemas/MatisseParametersDictionnary.xsd \
    ../../../Config/schemas/JobParameters.xsd \
