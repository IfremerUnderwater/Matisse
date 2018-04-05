#-------------------------------------------------
#
# Project created by QtCreator 2013-10-23T11:01:57
#
#-------------------------------------------------

QT       += core xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Module1
CONFIG += plugin

TEMPLATE = lib


#win32 {
#    QMAKE_CXXFLAGS += /wd4100 /wd4996
#    DEFINES += _USE_MATH_DEFINES
#}

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
include(../../Scripts/BasicProcessing.pri)
include(../../Scripts/OpticalMapping.pri)
include(../../Scripts/opencv.pri)
#include(../../Scripts/qgis.pri)

win32 {
    DLLDESTDIR = $${BUILD_DIR}/Modules/processors
}
unix {
    DESTDIR = $${BUILD_DIR}/Modules/processors
}

SOURCES += src/Module1.cpp

HEADERS += src/Module1.h
