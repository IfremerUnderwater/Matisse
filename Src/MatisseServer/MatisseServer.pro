#-------------------------------------------------
#
# Project created by QtCreator 2013-10-31T14:34:04
#
#-------------------------------------------------

TARGET = MatisseServer
TEMPLATE = app

QT       += core gui network sql xml xmlpatterns script

win32 {
    RC_FILE = MatisseServer.rc
    QMAKE_CXXFLAGS += /wd4100 /wd4996

    DEFINES += GUI_EXPORT=__declspec(dllimport) CORE_EXPORT=__declspec(dllimport)
}
else {
    message ("Set include...")
    INCLUDEPATH += /usr/include/qgis
    DEFINES += GUI_EXPORT= CORE_EXPORT=
    QMAKE_CXXFLAGS = -fpermissive

}

# Workaround to be removed in qt5 with qmake.conf and shadowed function --
SOURCE_DIR=$$PWD/../
CONFIG(debug, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Debug
    RUN_DIR=$${SOURCE_DIR}../Run/Debug
}
CONFIG(release, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Release
    RUN_DIR=$${SOURCE_DIR}../Run/Release
}
# ------------------------------------------------------------------------

DESTDIR = $${RUN_DIR}

include(../Scripts/MatisseCommon.pri)
include(../Scripts/MatisseTools.pri)
include(../Scripts/opencv.pri)
include(../Scripts/qgis.pri)

SOURCES += src/main.cpp\
    src/AssemblyGui.cpp \
    src/ProcessorWidget.cpp \
    src/SourceWidget.cpp \
    src/ElementWidget.cpp \
    src/DestinationWidget.cpp \
    src/ParametersWidget.cpp \
    src/AssemblyGraphicsScene.cpp \
    src/PipeWidget.cpp \
    src/AssemblyDialog.cpp \
    src/ExpertFormWidget.cpp \
    src/UserFormWidget.cpp \
    src/MatisseSpinBox.cpp \
    src/ParametersDialog.cpp \
    src/Server.cpp \
    src/JobDialog.cpp \
    src/StatusMessageWidget.cpp

HEADERS  += \
    src/AssemblyGui.h \
    src/ProcessorWidget.h \
    src/SourceWidget.h \
    src/ElementWidget.h \
    src/DestinationWidget.h \
    src/ParametersWidget.h \
    src/AssemblyGraphicsScene.h \
    src/PipeWidget.h \
    src/AssemblyDialog.h \
    src/ExpertFormWidget.h \
    src/UserFormWidget.h \
    src/MatisseSpinBox.h \
    src/ParametersDialog.h \
    src/Server.h \
    src/JobDialog.h \
    src/StatusMessageWidget.h

FORMS    += \
    ui/AssemblyGui.ui \
    ui/AssemblyDialog.ui \
    ui/ExpertFormWidget.ui \
    ui/UserFormWidget.ui \
    ui/ParametersDialog.ui \
    ui/ServerDialog.ui \
    ui/JobDialog.ui \
    ui/JobDialog.ui \
    ui/StatusMessageWidget.ui


TRANSLATIONS=MatisseServer_en.ts

RESOURCES += \
    ui/resources/resources.qrc \
    ui/style.qrc
