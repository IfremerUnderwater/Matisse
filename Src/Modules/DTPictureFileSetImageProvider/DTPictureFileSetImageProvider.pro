#-------------------------------------------------
#
# Project created by QtCreator 2014-03-13T19:10:30
#
#-------------------------------------------------
TEMPLATE = lib

QT       += core xml

TARGET = DTPictureFileSetImageProvider
CONFIG += plugin


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

win32 {
    DLLDESTDIR = $${BUILD_DIR}/Modules/imageProviders
}
unix {
    DESTDIR = $${BUILD_DIR}/Modules/imageProviders
}


SOURCES += \
    src/DTPictureFileSetImageProvider.cpp

HEADERS += \
    src/DTPictureFileSetImageProvider.h


