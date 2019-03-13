#-------------------------------------------------
#
# Project created by QtCreator 2014-04-30T09:31:01
#
#-------------------------------------------------

QT       += core xml

QMAKE_CXXFLAGS += /wd4100 /wd4996

TARGET = MatlabCppInterface
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += "$$(MATLAB_LIB_DIR)/include"
LIBS +=  -L"$$(MATLAB_LIB_DIR)/lib/win64/microsoft"
LIBS += -lmclmcrrt

INCLUDEPATH += ../../MatisseCommon/src  $$(OPENCV_DIR)/../../include
LIBS +=  -L$$(OPENCV_DIR)/lib


win32:release {
    message ("Compil release...")
    DESTDIR = ../../libs/release
    LIBS += -L../../libs/release
}

win32:debug {
    message ("Compil debug...")
    DESTDIR = ../../libs/debug
    LIBS += -L../../libs/debug
}

SOURCES += matlabcppinterface.cpp

HEADERS += matlabcppinterface.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
