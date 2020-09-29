QT += core
QT -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TestImagePreprocessing
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

win32 {
    DEFINES += _USE_MATH_DEFINES
}

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

include(../../Scripts/ImageProcessing.pri)
include(../../Scripts/BasicProcessing.pri)
include(../../Scripts/opencv.pri)


SOURCES += main.cpp

