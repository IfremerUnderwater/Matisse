#-------------------------------------------------
#
# Project created by QtCreator 2014-03-24T14:14:20
#
#-------------------------------------------------

QT       += core xml xmlpatterns network gui


TARGET = TestJob
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../../../main/qt/MatisseTools/src ../../../main/qt/MatisseCommon/src

win32:Release {
    LIBS += -L../../../main/qt/libs/release
    POST_TARGETDEPS += ../../../main/qt/libs/release/MatisseCommon.lib
    POST_TARGETDEPS += ../../../main/qt/libs/release/MatisseTools.lib
}

win32:Debug {
    LIBS += -L../../../main/qt/libs/debug
    POST_TARGETDEPS += ../../../main/qt/libs/debug/MatisseCommon.lib
    POST_TARGETDEPS += ../../../main/qt/libs/debug/MatisseTools.lib
}

LIBS += -lMatisseCommon -lMatisseTools

SOURCES += main.cpp

