#-------------------------------------------------
#
# Project created by QtCreator 2014-05-19T17:11:08
#
#-------------------------------------------------

QT += core xml network

win32{
QMAKE_CXXFLAGS += /wd4100 /wd4996
}

TARGET = RTPictureFileSetImageProvider
CONFIG += plugin

TEMPLATE = lib


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
    DLLDESTDIR = $${BUILD_DIR}/Modules/imageProviders
}
unix {
    DESTDIR = $${BUILD_DIR}/Modules/imageProviders
}

SOURCES += src/RTPictureFileSetImageProvider.cpp \
    src/Worker.cpp

HEADERS += src/RTPictureFileSetImageProvider.h \
    src/Worker.h
