win32 {
    INCLUDEPATH *= $$(OSGEO4W_ROOT)/include
    INCLUDEPATH *= $$(OSGEO4W_ROOT)/apps/qgis/include
    LIBS *= -L$$(OSGEO4W_ROOT)/lib
    LIBS *= -L$$(OSGEO4W_ROOT)/apps/qgis/lib
    DEFINES += GUI_EXPORT=__declspec(dllimport) CORE_EXPORT=__declspec(dllimport)
    message("OSGEO4W_ROOT/lib = $$(OSGEO4W_ROOT)/lib")
}
else {

    INCLUDEPATH += /usr/include/qgis
    DEFINES += GUI_EXPORT= CORE_EXPORT=

}
CONFIG(debug, debug|release) {
        LIBEXT=d
}
else {
        LIBEXT=""
}

win32{
    LIBS*= -lqgis_core$${LIBEXT} -lqgis_gui$${LIBEXT}
}
else{
    LIBS*= -lqgis_core -lqgis_gui
}

win32{
    LIBS*=-lproj_i -lgdal_i
}
else{
    LIBS*=-lproj
}
