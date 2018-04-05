include(libsOpenCVAddLibrary.pri)

win32 {
	INCLUDEPATH *=  $$(OPENCV_DIR)/../../include
#	LIBEXT=248
        LIBEXT=331

        CONFIG(debug, debug|release) {
                LIBEXT=$${LIBEXT}d
        }
        else {
                LIBEXT=$${LIBEXT}""
        }
#        LIBS  *= -L$$(OPENCV_DIR)/lib
        LIBS  *= /LIBPATH:$$(OPENCV_DIR)/lib

        message("OPENCV $$(OPENCV_DIR)")
}

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
#OpenCV 3.3.1
    LibsAddLibrary(world$$LIBEXT)
}
