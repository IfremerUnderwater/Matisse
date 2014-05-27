#-------------------------------------------------
#
# Project created by QtCreator 2013-10-31T14:34:04
#
#-------------------------------------------------

QT       += core gui network sql xml xmlpatterns script
#CONFIG += console
win32 {
    RC_FILE = MatisseServer.rc
    QMAKE_CXXFLAGS += /wd4100 /wd4996
}

TARGET = MatisseServer
TEMPLATE = app

INCLUDEPATH +=  $$(OPENCV_DIR)/../../include
INCLUDEPATH += $$(OSGEO4W_ROOT)/apps/qgis/include

LIBS +=  -L$$(OPENCV_DIR)/lib
LIBS += -L$$(OSGEO4W_ROOT)/apps/qgis/lib

message ("PATH" + $$INCLUDEPATH)
win32:Release {
    message ("Compil release...")

    LIBS += -L../libs/release
    LIBS += -lqgis_core -lqgis_gui
    LIBS += -lopencv_core248
    LIBS += -lopencv_highgui248
    LIBS += -lopencv_imgproc248d
    POST_TARGETDEPS += ../libs/release/MatisseCommon.lib ../libs/release/MatisseTools.lib
}

win32:Debug {
    message ("Compil debug...")
    LIBS += -L../libs/debug
    LIBS += -lqgis_cored -lqgis_guid
    LIBS += -lopencv_core248d
    LIBS += -lopencv_highgui248d
    LIBS += -lopencv_imgproc248d
    POST_TARGETDEPS += ../libs/debug/MatisseCommon.lib ../libs/debug/MatisseTools.lib
}

DEFINES += GUI_EXPORT=__declspec(dllimport) CORE_EXPORT=__declspec(dllimport)


LIBS += -lMatisseCommon -lMatisseTools

INCLUDEPATH += src
INCLUDEPATH += ../MatisseTools/src \
               ../MatisseCommon/src

SOURCES += src/main.cpp\
    src/AssemblyGui.cpp \
    src/ProcessorWidget.cpp \
    src/SourceWidget.cpp \
    src/ElementWidget.cpp \
    src/DestinationWidget.cpp \
    src/ParametersWidget.cpp \
    src/AssemblyGraphicsScene.cpp \
    src/PipeWidget.cpp \
    src/AssemblyDialog.cpp \
    src/ExpertFormWidget.cpp \
    src/UserFormWidget.cpp \
    src/MatisseSpinBox.cpp \
    src/ParametersDialog.cpp \
    src/Server.cpp \
    src/JobDialog.cpp \
    src/StatusMessageWidget.cpp

HEADERS  += \
    src/AssemblyGui.h \
    src/ProcessorWidget.h \
    src/SourceWidget.h \
    src/ElementWidget.h \
    src/DestinationWidget.h \
    src/ParametersWidget.h \
    src/AssemblyGraphicsScene.h \
    src/PipeWidget.h \
    src/AssemblyDialog.h \
    src/ExpertFormWidget.h \
    src/UserFormWidget.h \
    src/MatisseSpinBox.h \
    src/ParametersDialog.h \
    src/Server.h \
    src/JobDialog.h \
    src/StatusMessageWidget.h

FORMS    += \
    ui/AssemblyGui.ui \
    ui/AssemblyDialog.ui \
    ui/ExpertFormWidget.ui \
    ui/UserFormWidget.ui \
    ui/ParametersDialog.ui \
    ui/ServerDialog.ui \
    ui/JobDialog.ui \
    ui/JobDialog.ui \
    ui/StatusMessageWidget.ui

OBJECTS_DIR = temp
MOC_DIR = temp
UI_DIR = temp
RCC_DIR = temp

TRANSLATIONS=MatisseServer_en.ts

RESOURCES += \
    ui/resources/resources.qrc
