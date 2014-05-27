#-------------------------------------------------
#
# Project created by QtCreator 2013-10-17T10:34:00
#
#-------------------------------------------------

QT       += core gui xml script

TARGET = StructureFiller
TEMPLATE = app

UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp

INCLUDEPATH += ../.././main/qt/MatisseCommon/src
INCLUDEPATH += ../.././main/qt/MatisseTools/src

win32:release {
LIBS += -LC:/WorkspaceMatisse/main/qt/libs/release
}

win32:debug {
LIBS += -LC:/WorkspaceMatisse/main/qt/libs/debug
}

LIBS +=  -lMatisseCommon -lMatisseTools

SOURCES += src/main.cpp\
        src/StructureFillerGui.cpp

HEADERS  += src/StructureFillerGui.h

FORMS    += ui/StructureFillerGui.ui

RESOURCES += ui/resources/png.qrc
