#-------------------------------------------------
#
# Project created by QtCreator 2014-05-19T17:11:08
#
#-------------------------------------------------

QT += core xml network

QMAKE_CXXFLAGS += /wd4100 /wd4996

TARGET = RTPictureFileSetImageProvider
CONFIG += plugin

TEMPLATE = lib


UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp

INCLUDEPATH += ../../MatisseCommon/src $$(OPENCV_DIR)/../../include
LIBS +=  -L$$(OPENCV_DIR)/lib

win32:Release {
    LIBS += -L../../libs/release
    LIBS += -lopencv_core248
    LIBS += -lopencv_highgui248
    POST_TARGETDEPS += ../../libs/release/MatisseCommon.lib
}

win32:Debug {
    LIBS += -L../../libs/debug
    LIBS += -lopencv_core248d
    LIBS += -lopencv_highgui248d
    POST_TARGETDEPS += ../../libs/debug/MatisseCommon.lib
}

LIBS +=  -lMatisseCommon


DLLDESTDIR = ../../dll/imageProviders

SOURCES += src/RTPictureFileSetImageProvider.cpp \
    src/Worker.cpp

HEADERS += src/RTPictureFileSetImageProvider.h \
    src/Worker.h
