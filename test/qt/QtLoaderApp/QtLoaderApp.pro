#-------------------------------------------------
#
# Project created by QtCreator 2014-01-07T18:30:46
#
#-------------------------------------------------

QT       += core xml


TARGET = QtLoaderApp
CONFIG   += console
CONFIG   -= app_bundle
INCLUDEPATH += ../../../main/qt/MatisseCommon/src  $$(OPENCV_DIR)/../../include
LIBS +=  -L$$(OPENCV_DIR)/lib

win32:Release {
    LIBS += -L../../../main/qt/libs/release
    LIBS += -lopencv_core248
    LIBS += -lopencv_highgui248
    POST_TARGETDEPS += ../../../main/qt/libs/release/MatisseCommon.lib
}

win32:Debug {
    LIBS += -L../../../main/qt/libs/debug
    LIBS += -lopencv_core248d
    LIBS += -lopencv_highgui248d
    POST_TARGETDEPS += ../../../main/qt/libs/debug/MatisseCommon.lib
}


LIBS +=  -lMatisseCommon


TEMPLATE = app


SOURCES += main.cpp
