QT += xml network

win32 {
    QMAKE_CXXFLAGS += /wd4100 /wd4996
    INCLUDEPATH +=  $$(OPENCV_DIR)/../../include
    LIBS +=  -L$$(OPENCV_DIR)/lib
}


TARGET = MatisseCommon
TEMPLATE = lib
CONFIG += staticlib

UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp



CONFIG(debug, debug|release) {
    message ("Compil debug...")
    DESTDIR = ../libs/debug
    LIBS += -L../libs/debug
}
else {
    message ("Compil release...")
    DESTDIR = ../libs/release
    LIBS += -L../libs/release
}


HEADERS += \
    src/Context.h \
    src/Image.h \
    src/ImageListener.h \
    src/ImageSet.h \
    src/Processor.h \
    src/NavImage.h \
    src/NavInfo.h \
    src/ImageProvider.h \
    src/Dim2FileReader.h \
    src/PictureFileSet.h \
    src/InputSource.h \
    src/FileImage.h \
    src/Dim2.h \
    src/MatisseParameters.h \
    src/RasterProvider.h \
    src/MosaicContext.h \
    src/LifecycleComponent.h \
    src/Sleeper.h \
    src/Dim2UdpListener.h \
    src/GeoTransform.h

SOURCES += \
    src/Processor.cpp \
    src/NavInfo.cpp \
    src/ImageProvider.cpp \
    src/Dim2FileReader.cpp \
    src/PictureFileSet.cpp \
    src/Image.cpp \
    src/NavImage.cpp \
    src/ImageSet.cpp \
    src/Dim2.cpp \
    src/FileImage.cpp \
    src/MatisseParameters.cpp \
    src/RasterProvider.cpp \
    src/LifecycleComponent.cpp \
    src/Dim2UdpListener.cpp \
    src/GeoTransform.cpp


