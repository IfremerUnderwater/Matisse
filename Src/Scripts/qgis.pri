win32 {
    INCLUDEPATH *= $$(OSGEO4W_ROOT)/include
#    INCLUDEPATH *= $$(OSGEO4W_ROOT)/apps/qgis/include
    INCLUDEPATH *= "C:\Program Files\qgis2.99.0"/include
    LIBS *= -L$$(OSGEO4W_ROOT)/lib
#    LIBS *= -L$$(OSGEO4W_ROOT)/apps/qgis/lib
    LIBS *= -L"C:\Program Files\qgis2.99.0"/lib
    DEFINES += GUI_EXPORT=__declspec(dllimport) CORE_EXPORT=__declspec(dllimport)
    message("OSGEO4W_ROOT/lib = $$(OSGEO4W_ROOT)/lib")

#gdal
    INCLUDEPATH += F:\OSGeo4W64\apps\gdal-dev\include
    LIBS +=  -LF:\OSGeo4W64\apps\gdal-dev\lib\
}
else {

    INCLUDEPATH += /usr/include/qgis
    DEFINES += GUI_EXPORT= CORE_EXPORT=

}
CONFIG(debug, debug|release) {
        LIBEXT=""
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
message("qqis pri -- LIBS = $$LIBS")
