
QT       += core xml

TARGET = Init2DMosaic
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
include(../../Scripts/BasicProcessing.pri)
include(../../Scripts/OpticalMapping.pri)
include(../../Scripts/opencv.pri)
include(../../Scripts/qgis.pri)

win32 {
    DLLDESTDIR = $${BUILD_DIR}/Modules/processors
}
unix {
    DESTDIR = $${BUILD_DIR}/Modules/processors
}

SOURCES += src/Init2DMosaic.cpp

HEADERS += src/Init2DMosaic.h
