INCLUDEPATH += $${SOURCE_DIR}/Libraries/BasicProcessing

message("INCLUDEPATH in BasicProcessing pri = $$INCLUDEPATH")

CONFIG(debug, debug|release) {
    win32{
        LIBS += -L$${BUILD_DIR}/Libraries/BasicProcessing/debug
        message("BasicProcessing LIBS completion = $$LIBS")
    }
    else{
        LIBS *= -L$${BUILD_DIR}/Libraries/BasicProcessing
    }
}
else {
    win32{
        LIBS *= -L$${BUILD_DIR}/Libraries/BasicProcessing/release
    }
    else{
        LIBS *= -L$${BUILD_DIR}/Libraries/BasicProcessing
    }
}

LIBS*= -lBasicProcessing
