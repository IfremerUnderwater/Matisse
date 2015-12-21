include(libsOpenCVAddLibrary.pri)

win32 {
	INCLUDEPATH *=  $$(OPENCV_DIR)/../../include
	LIBEXT=248
}

unix {
        LIBEXT=""
}

#CONFIG(debug, debug|release) {
#	LIBEXT=$${LIBEXT}d
#}
#else {
#	LIBEXT=$${LIBEXT}""
#}

LibsAddLibrary(highgui$$LIBEXT)
LibsAddLibrary(core$$LIBEXT)
LibsAddLibrary(stitching$$LIBEXT)
LibsAddLibrary(imgproc$$LIBEXT)
