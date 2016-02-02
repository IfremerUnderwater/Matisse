INCLUDEPATH += $${SOURCE_DIR}/Libraries/MatisseCommon/src

message("INCLUDEPATH in MatisseCommon pri = $$INCLUDEPATH")

CONFIG(debug, debug|release) {
    win32{
        LIBS *= -L$${BUILD_DIR}/Libraries/MatisseCommon/debug
        PRE_TARGETDEPS += $${BUILD_DIR}/Libraries/MatisseCommon/debug
    }
    else{
        LIBS *= -L$${BUILD_DIR}/Libraries/MatisseCommon
    }
}
else {
    win32{
        LIBS *= -L$${BUILD_DIR}/Libraries/MatisseCommon/release
    }
    else{
        LIBS *= -L$${BUILD_DIR}/Libraries/MatisseCommon
    }
}

LIBS*= -lMatisseCommon

