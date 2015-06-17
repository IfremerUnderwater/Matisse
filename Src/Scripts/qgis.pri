win32 {
    INCLUDEPATH *= $$(OSGEO4W_ROOT)/include
    INCLUDEPATH *= $$(OSGEO4W_ROOT)/apps/qgis/include
    LIBS *= -L$$(OSGEO4W_ROOT)/lib
    LIBS *= -L$$(OSGEO4W_ROOT)/apps/qgis/lib
    message("OSGEO4W_ROOT/lib = $$(OSGEO4W_ROOT)/lib")
}

CONFIG(debug, debug|release) {
        LIBEXT=d
}
else {
	LIBEXT=""
}

LIBS*= -lqgis_core$${LIBEXT} -lqgis_gui$${LIBEXT}


win32{
    LIBS*=-lproj_i
}
else{
    LIBS*=-lproj
}
