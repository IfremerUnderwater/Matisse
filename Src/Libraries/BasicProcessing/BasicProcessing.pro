#-------------------------------------------------
#
# Project created by QtCreator 2015-07-23T15:34:05
#
#-------------------------------------------------

QT       -= gui

TARGET = BasicProcessing
TEMPLATE = lib
CONFIG += staticlib

SOURCES += Polygon.cpp \
           gpc.h

HEADERS += Polygon.h \
           gpc.c
unix {
    target.path = /usr/lib
    INSTALLS += target
}
