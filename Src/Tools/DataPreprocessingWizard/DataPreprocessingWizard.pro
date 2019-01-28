#-------------------------------------------------
#
# Project created by QtCreator 2018-12-21T09:19:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DataPreprocessingWizard
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
    data_preprocessing_wizard.cpp \
    interpolator.cpp \
    angle_interpolator.cpp \
    nav_file_reader.cpp

HEADERS += \
    data_preprocessing_wizard.h \
    interpolator.h \
    angle_interpolator.h \
    nav_file_reader.h

FORMS += \
    data_preprocessing_wizard.ui
