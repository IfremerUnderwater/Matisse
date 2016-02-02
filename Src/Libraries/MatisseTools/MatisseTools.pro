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


INCLUDEPATH += src

SOURCES +=\
    src/ChooseAndShow.cpp \
    src/ParametersWidgetSkeleton.cpp \
    src/KeyValueList.cpp \
    src/Tools.cpp \
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
    src/MatisseParametersManager.cpp

HEADERS +=\
          src/ChooseAndShow.h \
    src/ParametersWidgetSkeleton.h \
    src/KeyValueList.h \
    src/Tools.h \
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
    src/MatisseParametersManager.h

FORMS    += ui/ChooseAndShow.ui \
            ui/ParametersWidgetSkeleton.ui

TRANSLATIONS=MatisseTools_en.ts
