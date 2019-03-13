INCLUDEPATH += $${SOURCE_DIR}/Libraries/ImageProcessing

message("INCLUDEPATH in ImageProcessing pri = $$INCLUDEPATH")

CONFIG(debug, debug|release) {
    win32{
        LIBS *= -L$${BUILD_DIR}/Libraries/ImageProcessing/debug
    }
    else{
        LIBS *= -L$${BUILD_DIR}/Libraries/dll
    }

    LIBS*= -lImageProcessingd
}
else {
    win32{
        LIBS *= -L$${BUILD_DIR}/Libraries/ImageProcessing/release
    }
    else{
        LIBS *= -L$${BUILD_DIR}/Libraries/dll
    }

    LIBS*= -lImageProcessing
}
