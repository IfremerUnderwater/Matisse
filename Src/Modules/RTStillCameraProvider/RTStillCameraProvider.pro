
QT += core xml network

win32{
QMAKE_CXXFLAGS += /wd4100 /wd4996
}

QMAKE_CXXFLAGS += -std=c++11

TARGET = RTStillCameraProvider
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
include(../../Scripts/MatisseTools.pri)
include(../../Scripts/opencv.pri)
include(../../Scripts/BasicProcessing.pri)
include(../../Scripts/ProtoBuf.pri)
include(../../Scripts/qgis.pri)

win32 {
    DLLDESTDIR = $${BUILD_DIR}/Modules/imageProviders
    LIBS *= "C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Samples\multimedia\DirectShow\baseclasses\Release\strmbase.lib"

}
unix {
    DESTDIR = $${BUILD_DIR}/Modules/imageProviders
}

SOURCES += src/RTStillCameraProvider.cpp

HEADERS += src/RTStillCameraProvider.h
