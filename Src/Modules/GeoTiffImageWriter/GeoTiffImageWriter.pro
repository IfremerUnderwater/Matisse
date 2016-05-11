#-------------------------------------------------
#
# Project created by QtCreator 2014-03-13T19:10:30
#
#-------------------------------------------------

QT       += core xml

TARGET = GeoTiffImageWriter
CONFIG += plugin

TEMPLATE = lib

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
include(../../Scripts/BasicProcessing.pri)
include(../../Scripts/qgis.pri)

win32 {
    DLLDESTDIR = $${BUILD_DIR}/Modules/rasterProviders
}
unix {
    DESTDIR = $${BUILD_DIR}/Modules/rasterProviders
}

SOURCES += \
    src/GeoTiffImageWriter.cpp

HEADERS += \
    src/GeoTiffImageWriter.h


