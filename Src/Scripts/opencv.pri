include(libsOpenCVAddLibrary.pri)

win32 {
	INCLUDEPATH *=  $$(OPENCV_DIR)/../../include
	LIBEXT=248

        CONFIG(debug, debug|release) {
                LIBEXT=$${LIBEXT}d
        }
        else {
                LIBEXT=$${LIBEXT}""
        }
        LIBS  *= -L$$(OPENCV_DIR)/lib
}

unix {
        LIBEXT=""
}

LibsAddLibrary(highgui$$LIBEXT)
LibsAddLibrary(core$$LIBEXT)
LibsAddLibrary(stitching$$LIBEXT)
LibsAddLibrary(imgproc$$LIBEXT)

