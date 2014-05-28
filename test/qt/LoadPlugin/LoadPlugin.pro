#-------------------------------------------------
#
# Project created by QtCreator 2013-10-23T14:01:10
#
#-------------------------------------------------

QT       += core gui

TARGET = LoadPlugin
TEMPLATE = app

INCLUDEPATH += ../MatisseCommon

SOURCES += main.cpp\
        LoadPlugin.cpp \
    ChooseAndShow.cpp

HEADERS  += LoadPlugin.h \
    ../MatisseCommon/PluginInterface.h \
    ChooseAndShow.h

FORMS    += LoadPlugin.ui \
    ChooseAndShow.ui

RESOURCES += \
    resources/resources.qrc
