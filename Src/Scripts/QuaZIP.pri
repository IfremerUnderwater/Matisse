win32 {
    CONFIG(debug, debug|release) {
        LIBEXT="d"
    }
    else {
        LIBEXT=""
    }
}

unix {
    LIBEXT=""
}

LIBS *= -L$$(QUAZIP_DIR)/lib
LIBS *= -lquazip$${LIBEXT}

INCLUDEPATH *= $$(QUAZIP_DIR)/include

message("LIBS=" $$LIBS)
