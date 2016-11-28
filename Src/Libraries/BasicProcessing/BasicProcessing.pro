#-------------------------------------------------
#
# Project created by QtCreator 2015-07-23T15:34:05
#
#-------------------------------------------------

QMAKE_CXXFLAGS += -std=c++11

QT       -= gui

TARGET = BasicProcessing
TEMPLATE = lib
CONFIG += staticlib

SOURCES += Polygon.cpp \
    gpc.cpp \
    stdvectoperations.cpp

HEADERS += Polygon.h \
    gpc.h \
    stdvectoperations.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
