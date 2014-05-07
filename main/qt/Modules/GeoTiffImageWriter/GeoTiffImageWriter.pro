#-------------------------------------------------
#
# Project created by QtCreator 2014-03-13T19:10:30
#
#-------------------------------------------------

QT       += core xml

QMAKE_CXXFLAGS += /wd4100 /wd4996

TARGET = GeoTiffImageWriter
CONFIG += plugin

TEMPLATE = lib

UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp

INCLUDEPATH += ../../MatisseCommon/src $$(OPENCV_DIR)/../../include
LIBS +=  -L$$(OPENCV_DIR)/lib

win32:Release {
    LIBS += -L../../libs/release
    LIBS += -lopencv_core248
    LIBS += -lopencv_highgui248
    POST_TARGETDEPS += ../../libs/release/MatisseCommon.lib
}

win32:Debug {
    LIBS += -L../../libs/debug
    LIBS += -lopencv_core248d
    LIBS += -lopencv_highgui248d
    POST_TARGETDEPS += ../../libs/debug/MatisseCommon.lib
}

LIBS +=  -lMatisseCommon


DLLDESTDIR = ../../dll/rasterProviders

SOURCES += \
    src/GeoTiffImageWriter.cpp

HEADERS += \
    src/GeoTiffImageWriter.h


