include(libsOpenCVAddLibrary.pri)

#win32 {

#systemconf = $$(MSYSTEM)
#isEmpty ( systemconf ) {
#    message("OPENCV Win $$(MSYSTEM)")

#	INCLUDEPATH *=  $$(OPENCV_DIR)/../../include
##	LIBEXT=248
#        LIBEXT=331

#        CONFIG(debug, debug|release) {
#                LIBEXT=$${LIBEXT}d
#        }
#        else {
#                LIBEXT=$${LIBEXT}""
#        }
##        LIBS  *= -L$$(OPENCV_DIR)/lib
#        LIBS  *= /LIBPATH:$$(OPENCV_DIR)/lib

#        message("OPENCV $$(OPENCV_DIR)")
#}
#else {
# message("OPENCV $$(MSYSTEM)")
#}

#}

unix {
        LIBEXT=""
}

unix {
#opencv 2.4.8
    LibsAddLibrary(highgui$$LIBEXT)
    LibsAddLibrary(core$$LIBEXT)
    LibsAddLibrary(stitching$$LIBEXT)
    LibsAddLibrary(imgproc$$LIBEXT)
}

win32 {
LIBS *= -lopencv_highgui -lopencv_stitching  -lopencv_imgproc -lopencv_imgcodecs  -lopencv_core
}
