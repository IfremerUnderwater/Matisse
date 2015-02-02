#-------------------------------------------------
#
# Project created by QtCreator 2014-03-13T19:10:30
#
#-------------------------------------------------

QT       += core xml

TARGET = GeoTiffImageWriter
CONFIG += plugin

TEMPLATE = lib

UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp

win32 {
    QMAKE_CXXFLAGS += /wd4100 /wd4996
    INCLUDEPATH +=  $$(OPENCV_DIR)/../../include
    LIBS +=  -L$$(OPENCV_DIR)/lib
}

INCLUDEPATH += ../../MatisseCommon/src


win32 {
    CONFIG(debug, debug|release) {
        message ("Compil debug...")
        LIBS += -L../../libs/debug
        LIBS += -lopencv_core248d
        LIBS += -lopencv_highgui248d
        POST_TARGETDEPS += ../../libs/debug/MatisseCommon.lib
    }
    else {
        message ("Compil release...")
        LIBS += -L../../libs/release
        LIBS += -lopencv_core248
        LIBS += -lopencv_highgui248
        POST_TARGETDEPS += ../../libs/release/MatisseCommon.lib
    }
    DLLDESTDIR = ../../dll/rasterProviders
}
else {
    CONFIG(debug, debug|release) {
        message ("Compil debug...")
        # Not tested: missing debug version of libraries
        LIBS += -L../../libs/debug
        LIBS += -lopencv_cored
        LIBS += -lopencv_highguid
        POST_TARGETDEPS += ../../libs/debug/libMatisseCommon.a
    }
    else {
        message ("Compil release...")

        LIBS += -L../../libs/release
        LIBS += -lopencv_core
        LIBS += -lopencv_highgui
        POST_TARGETDEPS += ../../libs/release/libMatisseCommon.a
    }
    DESTDIR = ../../dll/rasterProviders
}



LIBS +=  -lMatisseCommon




SOURCES += \
    src/GeoTiffImageWriter.cpp

HEADERS += \
    src/GeoTiffImageWriter.h


