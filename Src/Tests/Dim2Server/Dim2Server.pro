#-------------------------------------------------
#
# Project created by QtCreator 2014-05-20T16:10:40
#
#-------------------------------------------------
TARGET = Dim2Server
TEMPLATE = app

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Workaround to be removed in qt5 with qmake.conf and shadowed function --
SOURCE_DIR=$$PWD/../../
CONFIG(debug, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Debug
}
CONFIG(release, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Release
}
# ------------------------------------------------------------------------

include(../../Scripts/MatisseCommon.pri)
include(../../Scripts/opencv.pri)


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

TRANSLATIONS=Dim2Server_en.ts Dim2Server_fr.ts

FORMS    += mainwindow.ui
