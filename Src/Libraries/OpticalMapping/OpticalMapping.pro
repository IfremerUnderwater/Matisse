#-------------------------------------------------
#
# Project created by QtCreator 2015-01-26T15:12:27
#
#-------------------------------------------------

QMAKE_CXXFLAGS += -std=c++11

#QT       -= gui
QT       += core xml script xmlpatterns network


TEMPLATE = lib

DEFINES += LIBOPTICALMAPPING_LIBRARY

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

TARGET = OpticalMapping$$BUILD_EXT
# ------------------------------------------------------------------------

include(../../Scripts/MatisseCommon.pri)
include(../../Scripts/MatisseTools.pri)
include(../../Scripts/opencv.pri)
include(../../Scripts/qgis.pri)
include(../../Scripts/QuaZIP.pri)
include(../../Scripts/BasicProcessing.pri)


win32 {
    DLLDESTDIR = $${BUILD_DIR}/Libraries/dll
}

unix {
    DESTDIR = $${BUILD_DIR}/Libraries/dll
}

SOURCES += \
    MosaicDrawer.cpp \
    MosaicOptimizer.cpp \
    MosaicDescriptor.cpp \
    ProjectiveCamera.cpp \
    FileImgExposureCompensate.cpp

HEADERS +=\
        libopticalmapping_global.h \
    MosaicDrawer.h \
    MosaicOptimizer.h \
    MosaicDescriptor.h \
    ProjectiveCamera.h \
    FileImgExposureCompensate.h \
    FileImgPrecomp.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
