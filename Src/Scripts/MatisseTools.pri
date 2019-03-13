INCLUDEPATH += $${SOURCE_DIR}/Libraries/MatisseTools/src

message("INCLUDEPATH in MatisseTools pri = $$INCLUDEPATH")

CONFIG(debug, debug|release) {
    win32{
        LIBS *= -L$${BUILD_DIR}/Libraries/MatisseTools/debug
        PRE_TARGETDEPS += $${BUILD_DIR}/Libraries/MatisseTools/debug
    }
    else{
        LIBS *= -L$${BUILD_DIR}/Libraries/MatisseTools
    }
}
else {
    win32{
        LIBS *= -L$${BUILD_DIR}/Libraries/MatisseTools/release
    }
    else{
        LIBS *= -L$${BUILD_DIR}/Libraries/MatisseTools
    }
}

LIBS*= -lMatisseTools

