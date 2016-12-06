QT += core
QT -= gui

TARGET = TestImagePreprocessing
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app



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
# ------------------------------------------------------------------------

include(../../Scripts/opencv.pri)
include(../../Scripts/BasicProcessing.pri)
include(../../Scripts/ImageProcessing.pri)


SOURCES += main.cpp

