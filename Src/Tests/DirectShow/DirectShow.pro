#-------------------------------------------------
#
# Project created by QtCreator 2014-06-24T17:44:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += $$(OPENCV_DIR)/../../include \
    "C:/Program Files/Microsoft SDKs/Windows/v7.1/Samples/multimedia/directshow/baseclasses" \
    "C:/Program Files/Microsoft SDKs/Windows/v7.1/Samples/multimedia/directshow/dmo/dmodemo"

LIBS += -LD:/Dev/Msvc/baseclasses/Debug
LIBS += -L"C:/Program Files/Microsoft SDKs/Windows/v7.1/Lib/x64"
LIBS += -L$$(OPENCV_DIR)/lib

win32:Debug {
    LIBS += -lstrmbasd
    LIBS += -lopencv_core248d
    LIBS += -lopencv_highgui248d
    LIBS += -lopencv_imgproc248d
    LIBS += -lopencv_objdetect248d
    LIBS += -lstrmiids
    LIBS += -lOle32


}


TARGET = DirectShow
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imageprocessor.cpp \
    sequenceprocessor.cpp \
    VMR_Capture.cpp

HEADERS  += mainwindow.h \
    imageprocessor.h \
    sequenceprocessor.h \
    VMR_Capture.h

FORMS    += mainwindow.ui
