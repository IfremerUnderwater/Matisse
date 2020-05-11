#-------------------------------------------------
#
# Project created by QtCreator 2013-10-31T14:34:04
#
#-------------------------------------------------

TARGET = MatisseCmd
TEMPLATE = app

QT       += core gui network sql xml xmlpatterns script opengl printsupport widgets



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
    src/JobLauncher.cpp \
    src/Server.cpp \
    src/CartoImage.cpp \

HEADERS  += \
    src/JobLauncher.h \
    src/Server.h \
    src/CartoImage.h \


OTHER_FILES += \
    ../../Config/config/MatisseSettings.xml

DISTFILES += \
    ../../Config/xml/models/MatisseAssembly.xsd \
    ../../Config/schemas/MatisseParametersDictionnary.xsd
