#-------------------------------------------------
#
# Project created by QtCreator 2013-10-23T14:01:10
# Display Matisse components informations
#
#-------------------------------------------------

QT       += core gui xml

TARGET = LoadPlugin
TEMPLATE = app

win32 {
    QMAKE_CXXFLAGS += /wd4100 /wd4996
    INCLUDEPATH +=  $$(OPENCV_DIR)/../../include
    LIBS +=  -L$$(OPENCV_DIR)/lib
}


INCLUDEPATH += ../../../main/qt/MatisseCommon/src

win32 {
    CONFIG(debug, debug|release) {
        message ("Compil debug...")
        LIBS += -L../../../main/qt/libs/debug
        LIBS += -lopencv_core248d
        LIBS += -lopencv_highgui248d
        POST_TARGETDEPS += ../../../main/qt/libs/debug/MatisseCommon.lib
    }
    else {
        message ("Compil release...")
        LIBS += -L../../../main/qt/libs/release
        LIBS += -lopencv_core248
        LIBS += -lopencv_highgui248
        POST_TARGETDEPS += ../../../main/qt/libs/release/MatisseCommon.lib
    }
}

LIBS +=  -lMatisseCommon

SOURCES += main.cpp\
        LoadPlugin.cpp

HEADERS  += LoadPlugin.h

FORMS    += LoadPlugin.ui

RESOURCES += \
    resources/resources.qrc
