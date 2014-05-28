#-------------------------------------------------
#
# Project created by QtCreator 2013-12-09T17:59:55
#
#-------------------------------------------------

QT       += script xml xmlpatterns network gui

QMAKE_CXXFLAGS += /wd4100 /wd4996
TARGET = MatisseTools
TEMPLATE = lib
CONFIG += staticlib

UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp

INCLUDEPATH += ../MatisseCommon/src

win32:release {
DESTDIR = ../libs/release
LIBS += -L../libs/release
}

win32:debug {
DESTDIR = ../libs/debug
LIBS += -L../libs/debug
}

LIBS +=  -lMatisseCommon

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
    src/EnrichedFileChooser.cpp

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
    src/ToolsCommon.h

FORMS    += ui/ChooseAndShow.ui \
            ui/ParametersWidgetSkeleton.ui

TRANSLATIONS=MatisseTools_en.ts
