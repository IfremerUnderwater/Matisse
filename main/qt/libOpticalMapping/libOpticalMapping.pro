#-------------------------------------------------
#
# Project created by QtCreator 2015-01-26T15:12:27
#
#-------------------------------------------------

#QT       -= gui
QT       += core xml

TARGET = libOpticalMapping
TEMPLATE = lib

DEFINES += LIBOPTICALMAPPING_LIBRARY

SOURCES += \
    MosaicDrawer.cpp \
    MosaicOptimizer.cpp \
    MosaicDescriptor.cpp \
    ProjectiveCamera.cpp

HEADERS +=\
        libopticalmapping_global.h \
    MosaicDrawer.h \
    MosaicOptimizer.h \
    MosaicDescriptor.h \
    ProjectiveCamera.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += ../MatisseCommon/src $$(OPENCV_DIR)/../../include
LIBS +=  -L$$(OPENCV_DIR)/lib
LIBS += -L$$(OSGEO4W_ROOT)/apps/qgis/lib

win32:Release {
    LIBS += -L../libs/release
    LIBS += -lopencv_core248
    LIBS += -lopencv_highgui248
    LIBS += -lopencv_stitching248
    LIBS += -lopencv_imgproc248
    LIBS += -lproj_i
    LIBS += -lMatisseCommon
message("Config Release")

}

win32:Debug {
    LIBS += -L../libs/debug
    LIBS += -lopencv_core248d
    LIBS += -lopencv_highgui248d
    LIBS += -lopencv_stitching248d
    LIBS += -lopencv_imgproc248d
    LIBS += -lproj_i
    LIBS += -lMatisseCommon
message("Config Debug")
}
