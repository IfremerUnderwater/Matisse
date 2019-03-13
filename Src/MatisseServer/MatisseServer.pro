#-------------------------------------------------
#
# Project created by QtCreator 2013-10-31T14:34:04
#
#-------------------------------------------------

TARGET = MatisseServer
TEMPLATE = app

QT       += core gui network sql xml xmlpatterns script opengl printsupport widgets

#win32 {
#    RC_FILE = MatisseServer.rc
#    QMAKE_CXXFLAGS += /wd4100 /wd4129 /wd4996
##    CONFIG(debug, debug|release) {
##        QMAKE_CXXFLAGS += /MDd
##    }

#    DEFINES += GUI_EXPORT=__declspec(dllimport) CORE_EXPORT=__declspec(dllimport)
#    DEFINES += _USE_MATH_DEFINES
#     LIBS*= -lKernel32
#}
#else {
#    message ("Set include...")
#    QMAKE_CXXFLAGS = -fpermissive -Wno-reorder

#}

#unix {
QMAKE_CXXFLAGS += -std=c++11
#}

#unix{
INCLUDEPATH *= $$PWD/src
#}

LIBS*= -lproj -lgdal

# Workaround to be removed in qt5 with qmake.conf and shadowed function --
SOURCE_DIR=$$PWD/../
CONFIG(debug, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Debug
    RUN_DIR=$${SOURCE_DIR}../Run/Debug
}
CONFIG(release, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Release
    RUN_DIR=$${SOURCE_DIR}../Run/Release
}
# ------------------------------------------------------------------------

DESTDIR = $${RUN_DIR}
DEFINES *= WITH_OSG

include(../Scripts/MatisseCommon.pri)
include(../Scripts/MatisseTools.pri)
include(../Scripts/opencv.pri)
#include(../Scripts/qgis.pri)
include(../Scripts/QuaZIP.pri)
include(../Scripts/BasicProcessing.pri)

include(../Scripts/OpenSceneGraph.pri)


SOURCES += src/main.cpp\
    src/AssemblyGui.cpp \
    src/ProcessorWidget.cpp \
    src/SourceWidget.cpp \
    src/ElementWidget.cpp \
    src/DestinationWidget.cpp \
    src/AssemblyGraphicsScene.cpp \
    src/PipeWidget.cpp \
    src/AssemblyDialog.cpp \
    src/ExpertFormWidget.cpp \
    src/UserFormWidget.cpp \
    src/ParametersDialog.cpp \
    src/Server.cpp \
    src/JobDialog.cpp \
    src/StatusMessageWidget.cpp \
    src/WelcomeDialog.cpp \
    src/HomeWidget.cpp \
    src/VisuModeWidget.cpp \
    src/MainMenuWidget.cpp \
    src/OngoingProcessWidget.cpp \
    src/MainControllBar.cpp \
    src/MatisseVersionWidget.cpp \
    src/MatisseTabWidget.cpp \
    src/MatisseMenu.cpp \
    src/PreferencesDialog.cpp \
    src/LiveProcessWheel.cpp \
    src/ParametersFoldButton.cpp \
    src/ElementWidgetProvider.cpp \
    src/AboutDialog.cpp \
    src/OSGWidget.cpp \
    src/DuplicateDialog.cpp \
    src/RestoreJobsDialog.cpp \
    src/NetworkCheckDialog.cpp \
    src/CartoScene.cpp \
    src/CartoImage.cpp \
    src/WheelGraphicsView.cpp

HEADERS  += \
    src/AssemblyGui.h \
    src/ProcessorWidget.h \
    src/SourceWidget.h \
    src/ElementWidget.h \
    src/DestinationWidget.h \
    src/AssemblyGraphicsScene.h \
    src/PipeWidget.h \
    src/AssemblyDialog.h \
    src/ExpertFormWidget.h \
    src/UserFormWidget.h \
    src/ParametersDialog.h \
    src/Server.h \
    src/JobDialog.h \
    src/StatusMessageWidget.h \
    src/WelcomeDialog.h \
    src/HomeWidget.h \
    src/VisuModeWidget.h \
    src/MainMenuWidget.h \
    src/OngoingProcessWidget.h \
    src/MainControllBar.h \
    src/MatisseVersionWidget.h \
    src/MatisseTabWidget.h \
    src/MatisseMenu.h \
    src/PreferencesDialog.h \
    src/LiveProcessWheel.h \
    src/ParametersFoldButton.h \
    src/ElementWidgetProvider.h \
    src/AboutDialog.h \
    src/OSGWidget.h \
    src/DuplicateDialog.h \
    src/RestoreJobsDialog.h \
    src/NetworkCheckDialog.h \
    src/CartoScene.h \
    src/CartoImage.h \
    src/WheelGraphicsView.h

FORMS    += \
    ui/AssemblyGui.ui \
    ui/AssemblyDialog.ui \
    ui/ExpertFormWidget.ui \
    ui/UserFormWidget.ui \
    ui/ParametersDialog.ui \
    ui/ServerDialog.ui \
    ui/JobDialog.ui \
    ui/StatusMessageWidget.ui \
    ui/WelcomeDialog.ui \
    ui/HomeWidget.ui \
    ui/VisuModeWidget.ui \
    ui/MainMenuWidget.ui \
    ui/OngoingProcessWidget.ui \
    ui/MainControllBar.ui \
    ui/MatisseVersionWidget.ui \
    ui/PreferencesDialog.ui \
    ui/AboutDialog.ui \
    ui/DuplicateDialog.ui \
    ui/RestoreJobsDialog.ui \
    ui/NetworkCheckDialog.ui


TRANSLATIONS=MatisseServer_fr.ts \
             MatisseServer_en.ts

RESOURCES += \
    ui/resources/resources.qrc \
    ui/style.qrc \
    ui/icons.qrc

OTHER_FILES += \
    ../../Config/lnf/Matisse.css \
    ../../Config/lnf/MatisseModeProg.css \
    ../../Config/lnf/MatisseModeRt.css \
    ../../Config/lnf/MatisseModeDt.css \
    ../../Config/lnf/MatisseColorsDay.properties \
    ../../Config/lnf/MatisseColorsNight.properties \
    MatisseServer_fr.ts \
    MatisseServer_en.ts \
    ../../Config/config/MatisseSettings.xml

DISTFILES += \
    ../../Config/xml/models/MatisseAssembly.xsd \
    ../../Config/schemas/MatisseParametersDictionnary.xsd


RC_ICONS = ./ui/resources/ifremer-grand.ico
