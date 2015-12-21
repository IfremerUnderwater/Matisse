#-------------------------------------------------
#
# Project created by QtCreator 2014-01-21T16:28:57
#
#-------------------------------------------------

TARGET = IntegQGis
TEMPLATE = app

QT       += core gui xml

QGIS_DIR = $$(OSGEO4W_ROOT)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += GUI_EXPORT=__declspec(dllimport) CORE_EXPORT=__declspec(dllimport)

# Workaround to be removed in qt5 with qmake.conf and shadowed function --
SOURCE_DIR=$$PWD/../../
CONFIG(debug, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Debug
}
CONFIG(release, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Release
}
# ------------------------------------------------------------------------

include(../../Scripts/qgis.pri)

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += \
    mainwindowbase.ui

RESOURCES += \
    resources.qrc
