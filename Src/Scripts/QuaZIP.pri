
win32 {
LIBS *= -lquazip5
}

unix {
LIBS *= -lquazip
}

message("LIBS=" $$LIBS)
