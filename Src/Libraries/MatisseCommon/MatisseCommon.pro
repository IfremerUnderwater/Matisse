TARGET = MatisseCommon
TEMPLATE = lib
CONFIG += staticlib

QT += core xml network

win32 {
    QMAKE_CXXFLAGS += /wd4100 /wd4996
}

# Workaround to be removed in qt5 with qmake.conf and shadowed function --
SOURCE_DIR=$$PWD/../../
CONFIG(debug, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Debug
}
CONFIG(release, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Release
}

include(../../Scripts/opencv.pri)
include(../../Scripts/qgis.pri)
include(../../Scripts/BasicProcessing.pri)

PROTOS = src/proto/nav_photo_info.proto
include(../../Scripts/ProtoBuf.pri)


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
    src/GeoTransform.h \
    src/RasterGeoreferencer.h \
    src/GraphicalCharter.h \
    src/NavPhotoInfoTcpListener.h \
    src/matissemetatypes.h

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
    src/GeoTransform.cpp \
    src/RasterGeoreferencer.cpp \
    src/NavPhotoInfoTcpListener.cpp

OTHER_FILES += src/proto/*.*




