#-------------------------------------------------
#
# Project created by QtCreator 2013-10-23T11:01:57
#
#-------------------------------------------------

QT       += core xml
QMAKE_CXXFLAGS += /wd4100 /wd4996
TARGET = Module1
CONFIG += plugin

TEMPLATE = lib

UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp

INCLUDEPATH += ../../../main/qt/MatisseCommon/src  $$(OPENCV_DIR)/../../include
LIBS +=  -L$$(OPENCV_DIR)/lib

win32:Release {
    LIBS += -L../../../main/qt/libs/release
    LIBS += -lopencv_core248
    LIBS += -lopencv_highgui248
    POST_TARGETDEPS += ../../../main/qt/libs/release/MatisseCommon.lib
}

win32:Debug {
    LIBS += -L../../../main/qt/libs/debug
    LIBS += -lopencv_core248d
    LIBS += -lopencv_highgui248d
    POST_TARGETDEPS += ../../../main/qt/libs/debug/MatisseCommon.lib
}

LIBS +=  -lMatisseCommon


DLLDESTDIR = ../../../main/qt/dll/processors


SOURCES += src/Module1.cpp

HEADERS += src/Module1.h

