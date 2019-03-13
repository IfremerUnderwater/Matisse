
#win32 {
#        INCLUDEPATH *=  $$(OSG_DIR)/include
#        INCLUDEPATH *=  $$(OSG_DIR)/../include
##        LIBS += -L$$(OSG_DIR)/lib
#        LIBS  *= -L$$(OSG_DIR)/lib

#        CONFIG(debug, debug|release) {
#                LIBEXT=d
#        }
#        else {
## release with debug info
##                LIBEXT="rd"
#        LIBEXT=""
#        }
#}

#unix {
        LIBEXT=""
#}

message("Adding Osg dep.")
message("OSG_DIR $$(OSG_DIR)" )
message("LIBS=" $$LIBS)

# osg lib link
LIBS += -lOpenThreads$$LIBEXT -losg$$LIBEXT -losgGA$$LIBEXT -losgDB$$LIBEXT -losgUtil$$LIBEXT -losgViewer$$LIBEXT

