#-------------------------------------------------
#
# Project created by QtCreator 2013-10-23T11:01:57
#
#-------------------------------------------------

QT       += core xml

TARGET = Module1
CONFIG += plugin

TEMPLATE = lib

UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp

win32 {
    QMAKE_CXXFLAGS += /wd4100 /wd4996
    INCLUDEPATH +=  $$(OPENCV_DIR)/../../include
    INCLUDEPATH += $$(OSGEO4W_ROOT)/apps/qgis/include
    LIBS +=  -L$$(OPENCV_DIR)/lib
    LIBS += -L$$(OSGEO4W_ROOT)/apps/qgis/lib
}

INCLUDEPATH += ../../../main/qt/MatisseCommon/src
INCLUDEPATH += ../../../main/qt/libOpticalMapping

win32 {
    CONFIG(debug, debug|release) {
        message ("Compil debug...")
        LIBS += -L../../../main/qt/libs/debug
        LIBS += -L../../../main/qt/libOpticalMapping/debug
        LIBS += -lopencv_core248d
        LIBS += -lopencv_highgui248d
        LIBS += -lproj
        LIBS += -lMatisseCommon
        LIBS += -llibOpticalMapping
    }
    else {
        message ("Compil release...")
        LIBS += -L../../../main/qt/libs/release
        LIBS += -L../../../main/qt/libOpticalMapping/release
        LIBS += -lopencv_core248
        LIBS += -lopencv_highgui248
        LIBS += -lproj
        LIBS += -lMatisseCommon
        LIBS += -llibOpticalMapping
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

SOURCES += src/Module1.cpp

HEADERS += src/Module1.h
