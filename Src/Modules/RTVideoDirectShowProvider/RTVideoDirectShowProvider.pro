#-------------------------------------------------
#
# Project created by QtCreator 2014-03-13T19:10:30
#
#-------------------------------------------------

# Only works for Windows (DirectShow!)

QT       += core gui xml network widgets


TARGET = RTVideoDirectShowProvider
CONFIG += plugin

TEMPLATE = lib

win32{

# Workaround to be removed in qt5 with qmake.conf and shadowed function --
SOURCE_DIR=$$PWD/../../
CONFIG(debug, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Debug
}
CONFIG(release, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Release
}

QMAKE_CXXFLAGS += -std=c++11

# ------------------------------------------------------------------------

include(../../Scripts/MatisseCommon.pri)
include(../../Scripts/opencv.pri)
include(../../Scripts/BasicProcessing.pri)
include(../../Scripts/qgis.pri)

DLLDESTDIR = $${BUILD_DIR}/Modules/imageProviders



QMAKE_CXXFLAGS += /wd4100 /wd4996
INCLUDEPATH += "$$(WINDOWS_SDK_ROOT)/Samples/multimedia/directshow/baseclasses" \
INCLUDEPATH += "$$(WINDOWS_SDK_ROOT)/Samples/multimedia/directshow/dmo/dmodemo"

#LIBS += -L"C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1/Samples/multimedia/directshow/baseclasses/Release"
#LIBS += -L"C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1/Lib/x64"

LIBS += -L"$$(WINDOWS_SDK_ROOT)/Samples/multimedia/directshow/baseclasses/Release"
LIBS += -L"$$(WINDOWS_SDK_ROOT)/Lib/x64"


CONFIG(debug, debug|release) {
    message ("Compil debug...")
    LIBS += -lstrmiids
    LIBS += -lstrmbase
    LIBS += -lOle32
}
else {
    message ("Compil release...")
    LIBS += -lstrmiids
    LIBS += -lstrmbase
    LIBS += -lOle32
}



SOURCES += \
    src/CameraVideoSource.cpp \
    src/RTVideoDirectShowProvider.cpp \
    src/WorkerVideo.cpp \
    src/VMR_Capture.cpp \
    src/videopreview.cpp

HEADERS += \
    src/CameraVideoSource.h \
    src/WorkerVideo.h \
    src/RTVideoDirectShowProvider.h \
    src/VMR_Capture.h \
    src/videopreview.h

FORMS += \
    ui/videopreview.ui

}

