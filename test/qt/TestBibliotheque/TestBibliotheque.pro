#-------------------------------------------------
#
# Project created by QtCreator 2013-10-23T11:01:57
#
#-------------------------------------------------

QT       += core gui xml

TARGET = TestBibliotheque

TEMPLATE = lib

UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp

INCLUDEPATH += ../../../main/qt/MatisseCommon/src

win32:release {
LIBS += -L../../../main/qt/libs/release
}

win32:debug {
LIBS += -L../../../main/qt/libs/debug
}

LIBS +=  -lMatisseCommon

DLLDESTDIR = ../../../main/qt/modules

CONFIG += plugin

SOURCES += src/ModuleImplementation.cpp \
    src/Test.cpp

HEADERS += src/ModuleImplementation.h

