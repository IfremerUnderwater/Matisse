QT       += core xml widgets

TARGET = ReconstructionLister
CONFIG += plugin

TEMPLATE = lib

#win32 {
#    QMAKE_CXXFLAGS += /wd4100 /wd4996
#    DEFINES += _USE_MATH_DEFINES
#}
#unix {
QMAKE_CXXFLAGS += -std=c++11
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
include(../../Scripts/opencv.pri)
include(../../Scripts/BasicProcessing.pri)

win32 {
    DLLDESTDIR = $${BUILD_DIR}/Modules/rasterProviders
}
unix {
    DESTDIR = $${BUILD_DIR}/Modules/rasterProviders
}

SOURCES += \
    src/ReconstructionLister.cpp

HEADERS += \
    src/ReconstructionLister.h

