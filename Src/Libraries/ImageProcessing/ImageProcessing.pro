#-------------------------------------------------
#
# Project created by QtCreator 2016-12-01T16:03:54
#
#-------------------------------------------------

#QT       -= gui

TEMPLATE = lib

DEFINES += IMAGEPROCESSING_LIBRARY

# Workaround to be removed in qt5 with qmake.conf and shadowed function --
SOURCE_DIR=$$PWD/../../
CONFIG(debug, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Debug
    BUILD_EXT="d"
}
CONFIG(release, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Release
    BUILD_EXT=""
}

TARGET = ImageProcessing$$BUILD_EXT
# ------------------------------------------------------------------------

include(../../Scripts/opencv.pri)
include(../../Scripts/BasicProcessing.pri)


win32 {
    DLLDESTDIR = $${BUILD_DIR}/Libraries/dll
}

unix {
    DESTDIR = $${BUILD_DIR}/Libraries/dll
}


SOURCES += imageprocessing.cpp

HEADERS += imageprocessing.h\
        imageprocessing_global.h


