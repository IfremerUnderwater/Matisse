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
    gpc.cpp

HEADERS += Polygon.h \
    gpc.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
