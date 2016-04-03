
win32 {
	INCLUDEPATH *=  $$(OSG_DIR)/include
        LIBS += -L$$(OSG_DIR)/lib
        LIBS += -lOpenThreads
}

message("Adding Osg dep.")
message($$LIBS)

# osg lib link
LIBS += -losg -losgGA -losgDB -losgUtil -losgViewer

