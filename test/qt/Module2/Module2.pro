#-------------------------------------------------
#
# Project created by QtCreator 2013-10-23T11:01:57
#
#-------------------------------------------------

QT       += core xml
TARGET = Module2
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

INCLUDEPATH += ../../../main/qt/MatisseCommon/src

win32 {
    CONFIG(debug, debug|release) {
        message ("Compil debug...")
        LIBS += -L../../../main/qt/libs/debug
        LIBS += -lopencv_core248d
        LIBS += -lopencv_highgui248d
        POST_TARGETDEPS += ../../../main/qt/libs/debug/MatisseCommon.lib
    }
    else {
        message ("Compil release...")
        LIBS += -L../../../main/qt/libs/release
        LIBS += -lopencv_core248
        LIBS += -lopencv_highgui248
        POST_TARGETDEPS += ../../../main/qt/libs/release/MatisseCommon.lib
    }
    DLLDESTDIR = ../../../main/qt/dll/processors
}
else {
    CONFIG(debug, debug|release) {
        message ("Compil debug...")
        # Not tested: missing debug version of libraries
        LIBS += -L./../../main/qt/libs/debug
        LIBS += -lopencv_cored
        LIBS += -lopencv_highguid
        POST_TARGETDEPS += ../../../main/qt/libs/debug/libMatisseCommon.a
    }
    else {
        message ("Compil release...")

        LIBS += -L../../../main/qt/libs/release
        LIBS += -lopencv_core
        LIBS += -lopencv_highgui
        POST_TARGETDEPS += ../../../main/qt/libs/release/libMatisseCommon.a
    }
    DESTDIR = ../../../main/qt/dll/processors
}

LIBS +=  -lMatisseCommon

SOURCES += src/Module2.cpp

HEADERS += src/Module2.h
