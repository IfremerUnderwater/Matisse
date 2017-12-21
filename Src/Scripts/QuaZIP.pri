win32 {
    CONFIG(debug, debug|release) {
        LIBEXT="5d"
    }
    else {
        LIBEXT="5"
    }
}

unix {
    LIBEXT=""
}

LIBS *= -L$$(QUAZIP_DIR)/lib
LIBS *= -lquazip$${LIBEXT}

INCLUDEPATH *= $$(QUAZIP_DIR)/include

message("LIBS=" $$LIBS)
