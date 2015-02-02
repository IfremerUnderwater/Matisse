#-------------------------------------------------
#
# Project created by QtCreator 2013-10-23T11:01:57
#
#-------------------------------------------------

QT       += core xml

QMAKE_CXXFLAGS += /wd4100 /wd4996

TARGET = OptimizationModule
CONFIG += plugin

TEMPLATE = lib

UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp

INCLUDEPATH += "$$(MATLAB_LIB_DIR)"/include
LIBS +=  -L"$$(MATLAB_LIB_DIR)"/lib/win64/microsoft
LIBS += -lmclmcrrt

INCLUDEPATH += ../../MatisseCommon/src ../MatlabCppInterface  $$(OPENCV_DIR)/../../include
LIBS +=  -L$$(OPENCV_DIR)/lib

INCLUDEPATH += ../MatisseCppLib/distrib
LIBS +=  -L../MatisseCppLib/distrib
LIBS +=-lMatisseCppLib


win32:Release {
    LIBS += -L../../libs/release
    LIBS += -lopencv_core248
    LIBS += -lopencv_highgui248
    LIBS += -lMatlabCppInterface
    POST_TARGETDEPS += ../../libs/release/MatisseCommon.lib
}

win32:Debug {
    LIBS += -L../../libs/debug
    LIBS += -lopencv_core248d
    LIBS += -lopencv_highgui248d
    LIBS += -lMatlabCppInterface
    POST_TARGETDEPS += ../../libs/debug/MatisseCommon.lib
}


LIBS +=  -lMatisseCommon


DLLDESTDIR = ../../dll/processors


SOURCES += src/OptimizationModule.cpp

HEADERS += src/OptimizationModule.h

