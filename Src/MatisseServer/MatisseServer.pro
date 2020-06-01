#-------------------------------------------------
#
# Project created by QtCreator 2013-10-31T14:34:04
#
#-------------------------------------------------

TARGET = MatisseServer
TEMPLATE = app

QT       += core gui network xml xmlpatterns script opengl widgets

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
    src/OSGWidget/area_computation_visitor.cpp \
    src/OSGWidget/box_visitor.cpp \
    src/OSGWidget/geometry_type_count_visitor.cpp \
    src/OSGWidget/kml_handler.cpp \
    src/OSGWidget/measurement_picker_tool.cpp \
    src/OSGWidget/minmax_computation_visitor.cpp \
    src/OSGWidget/osg_widget.cpp \
    src/OSGWidget/osg_widget_tool.cpp \
    src/OSGWidget/overlay_widget.cpp \
    src/OSGWidget/point3d.cpp \
    src/OSGWidget/shader_color.cpp \
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
    src/DuplicateDialog.cpp \
    src/RestoreJobsDialog.cpp \
    src/NetworkCheckDialog.cpp \
    src/CartoScene.cpp \
    src/CartoImage.cpp \
    src/WheelGraphicsView.cpp

HEADERS  += \
    src/AssemblyGui.h \
    src/OSGWidget/area_computation_visitor.h \
    src/OSGWidget/box_visitor.h \
    src/OSGWidget/geometry_type_count_visitor.h \
    src/OSGWidget/kml_handler.h \
    src/OSGWidget/loading_mode.h \
    src/OSGWidget/measurement_picker_tool.h \
    src/OSGWidget/minmax_computation_visitor.h \
    src/OSGWidget/osg_widget.h \
    src/OSGWidget/osg_widget_tool.h \
    src/OSGWidget/overlay_widget.h \
    src/OSGWidget/point3d.h \
    src/OSGWidget/shader_color.h \
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
    src/DuplicateDialog.h \
    src/RestoreJobsDialog.h \
    src/NetworkCheckDialog.h \
    src/CartoScene.h \
    src/CartoImage.h \
    src/WheelGraphicsView.h

FORMS    += \
    src/AssemblyGui.ui \
    src/AssemblyDialog.ui \
    src/ExpertFormWidget.ui \
    src/UserFormWidget.ui \
    src/ParametersDialog.ui \
    src/ServerDialog.ui \
    src/JobDialog.ui \
    src/StatusMessageWidget.ui \
    src/WelcomeDialog.ui \
    src/HomeWidget.ui \
    src/VisuModeWidget.ui \
    src/MainMenuWidget.ui \
    src/OngoingProcessWidget.ui \
    src/MainControllBar.ui \
    src/MatisseVersionWidget.ui \
    src/PreferencesDialog.ui \
    src/AboutDialog.ui \
    src/DuplicateDialog.ui \
    src/RestoreJobsDialog.ui \
    src/NetworkCheckDialog.ui


TRANSLATIONS=MatisseServer_fr.ts \
             MatisseServer_en.ts

RESOURCES += \
    resources/resources.qrc \
    style.qrc \
    icons.qrc

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


RC_ICONS = ./resources/ifremer-grand.ico
