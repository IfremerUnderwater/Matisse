
win32 {
	INCLUDEPATH *=  $$(OSG_DIR)/include
        LIBS += -L$$(OSG_DIR)/lib

        CONFIG(debug, debug|release) {
                LIBEXT=d
        }
        else {
                LIBEXT=""
        }
}

unix {
        LIBEXT=""
}

message("Adding Osg dep.")

# osg lib link
LIBS += -lOpenThreads$$LIBEXT -losg$$LIBEXT -losgGA$$LIBEXT -losgDB$$LIBEXT -losgUtil$$LIBEXT -losgViewer$$LIBEXT

