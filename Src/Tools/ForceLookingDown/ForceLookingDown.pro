QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

#QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS+= -march=skylake -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mfma -mbmi2 -mavx2 -mno-sse4a -mno-xop -mno-fma4 -mno-avx512f -mno-avx512vl -mno-avx512pf -mno-avx512er -mno-avx512cd -mno-avx512dq -mno-avx512bw -mno-avx512ifma -mno-avx512vbmi -fopenmp -DOPENMVG_USE_AVX2 -pthread -std=gnu++11


QMAKE_LFLAGS +=  -fopenmp

#win32 {
#INCLUDEPATH += "C:/msys64/mingw64/include"
#INCLUDEPATH += "C:/msys64/mingw64/include/OpenMVG"
#INCLUDEPATH += C:\msys64\mingw64\include\eigen3
#
#}

LIBS *= -lopenMVG_image -lopenMVG_sfm  -lopenMVG_multiview -lopenMVG_exif -lopenMVG_stlplus -lopenMVG_features -lopenMVG_geometry -ljpeg -lpng -ltiff


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    force_looking_down_gui.cpp

HEADERS += \
    force_looking_down_gui.h

FORMS += \
    force_looking_down_gui.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
