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

UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp

win32 {
    QMAKE_CXXFLAGS += /wd4100 /wd4996
    INCLUDEPATH +=  $$(OPENCV_DIR)/../../include
    INCLUDEPATH += "$$(WINDOWS_SDK_ROOT)/Samples/multimedia/directshow/baseclasses" \
    INCLUDEPATH += "$$(WINDOWS_SDK_ROOT)/Samples/multimedia/directshow/dmo/dmodemo"

    LIBS +=  -L$$(OPENCV_DIR)/lib

    LIBS += -L"C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1/Samples/multimedia/directshow/baseclasses/Release"
    LIBS += -L"C:/Program Files (x86)/Microsoft SDKs/Windows/v7.1/Lib/x64"

} else {
    message("This component is for Windows only")
}

INCLUDEPATH += ../../MatisseCommon/src


win32 {
    CONFIG(debug, debug|release) {
        message ("Compil debug...")
        LIBS += -L../../libs/debug
        LIBS += -lopencv_core248d
        LIBS += -lopencv_highgui248d
        LIBS += -lstrmiids
        LIBS += -lstrmbase
        LIBS += -lOle32
        POST_TARGETDEPS += ../../libs/debug/MatisseCommon.lib
    }
    else {
        message ("Compil release...")
        LIBS += -L../../libs/release
        LIBS += -lopencv_core248
        LIBS += -lopencv_highgui248
        LIBS += -lstrmiids
        LIBS += -lstrmbase
        LIBS += -lOle32
        POST_TARGETDEPS += ../../libs/release/MatisseCommon.lib
    }
    DLLDESTDIR = ../../dll/imageProviders
}



LIBS +=  -lMatisseCommon


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


