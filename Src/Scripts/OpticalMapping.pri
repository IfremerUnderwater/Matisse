INCLUDEPATH += $${SOURCE_DIR}/Libraries/OpticalMapping

message("INCLUDEPATH in OpticalMapping pri = $$INCLUDEPATH")

CONFIG(debug, debug|release) {
    win32{
        LIBS *= -L$${BUILD_DIR}/Libraries/OpticalMapping/debug
    }
    else{
        LIBS *= -L$${BUILD_DIR}/Libraries/OpticalMapping
    }

    LIBS*= -lOpticalMappingd
}
else {
    win32{
        LIBS *= -L$${BUILD_DIR}/Libraries/OpticalMapping/release
    }
    else{
        LIBS *= -L$${BUILD_DIR}/Libraries/OpticalMapping
    }

    LIBS*= -lOpticalMapping
}
