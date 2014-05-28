#-------------------------------------------------
#
# Project created by QtCreator 2014-05-20T16:10:40
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += ../../../main/qt/MatisseCommon/src $$(OPENCV_DIR)/../../include
#LIBS +=  -L$$(OPENCV_DIR)/lib

win32:Release {
    LIBS += -L../../../main/qt/libs/release
#    LIBS += -lopencv_core248
#    LIBS += -lopencv_highgui248
#    POST_TARGETDEPS += ../../../main/qt/libs/release/MatisseCommon.lib
}

win32:Debug {
    LIBS += -L../../../main/qt/libs/debug
#    LIBS += -lopencv_core248d
#    LIBS += -lopencv_highgui248d
#    POST_TARGETDEPS += ../../../main/qt/libs/debug/MatisseCommon.lib
}

#LIBS +=  -lMatisseCommon

TARGET = Dim2Server
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

TRANSLATIONS=Dim2Server_en.ts Dim2Server_fr.ts

FORMS    += mainwindow.ui
