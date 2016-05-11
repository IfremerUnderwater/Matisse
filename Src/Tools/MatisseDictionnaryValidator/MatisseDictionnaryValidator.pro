#-------------------------------------------------
#
# Project created by QtCreator 2016-02-08T17:14:36
#
#-------------------------------------------------

QT       += core gui widgets xml xmlpatterns

TARGET = MatisseDictionnaryValidator
TEMPLATE = app

CONFIG(debug, debug|release) {
    RUN_DIR=$${PWD}/Run/Debug
}
CONFIG(release, debug|release) {
    RUN_DIR=$${PWD}/Run/Release
}
# ------------------------------------------------------------------------

DESTDIR = $${RUN_DIR}



SOURCES += src/main.cpp\
        src/DictionnaryValidator.cpp

HEADERS  += src/DictionnaryValidator.h

FORMS    += ui/DictionnaryValidator.ui

OTHER_FILES += config/MatisseParametersDictionnary.xml

message("Dest: $$DESTDIR")
message("Pwd: $$PWD")

confxml.path=$${DESTDIR}/config
confxml.files=$$PWD/config/*

INSTALLS    += confxml

