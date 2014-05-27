#-------------------------------------------------
#
# Project created by QtCreator 2014-01-21T16:28:57
#
#-------------------------------------------------

QT       += core gui xml

QGIS_DIR = $$(OSGEO4W_ROOT)

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IntegQGis
TEMPLATE = app

INCLUDEPATH += $$QGIS_DIR/apps/qgis/include

LIBS += -L$$QGIS_DIR/apps/qgis/lib

CONFIG(debug, debug|release) {
    LIBS += -lqgis_cored -lqgis_guid
} else {
    LIBS += -lqgis_core -lqgis_gui
}

DEFINES += GUI_EXPORT=__declspec(dllimport) CORE_EXPORT=__declspec(dllimport)


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += \
    mainwindowbase.ui

RESOURCES += \
    resources.qrc
