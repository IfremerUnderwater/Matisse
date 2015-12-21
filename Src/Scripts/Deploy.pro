# Workaround to be removed in qt5 with qmake.conf and shadowed function --
SOURCE_DIR=$$PWD/../
CONFIG_DIR=$${SOURCE_DIR}../Config
message("CONFIG_DIR = $$CONFIG_DIR")
CONFIG(debug, debug|release) {
    BUILD_DIR=$${SOURCE_DIR}../Build/Debug
    RUN_DIR=$${SOURCE_DIR}../Run/Debug
}else {
    BUILD_DIR=$${SOURCE_DIR}../Build/Release
    RUN_DIR=$${SOURCE_DIR}../Run/Release
}
# ------------------------------------------------------------------------

message("RUN_DIR = $$RUN_DIR")

win32{
    DEL_FILE      = del
    MKDIR         = mkdir
    COPY          = copy /y
    COPY_DIR      = xcopy /s /q /y /i
    DEL_DIR       = rmdir
    MOVE          = move
}

unix{
    DEL_FILE      = rm -f
    MKDIR         = mkdir
    COPY          = cp
    COPY_DIR      = cp -rf
    DEL_DIR       = rm -rf
    MOVE          = mv
}



# Create directories
system("$$MKDIR \"$$RUN_DIR/Modules\"")
system("$$MKDIR \"$$RUN_DIR/Modules/imageProviders\"")
system("$$MKDIR \"$$RUN_DIR/Modules/processors\"")
system("$$MKDIR \"$$RUN_DIR/Modules/rasterProviders\"")
system("$$MKDIR \"$$RUN_DIR/xml\"")

win32{
# Copy Modules in Run directory
system("$$COPY_DIR \"$$BUILD_DIR/Modules/imageProviders\" \"$$RUN_DIR/Modules/imageProviders\"")
system("$$COPY_DIR \"$$BUILD_DIR/Modules/processors\" \"$$RUN_DIR/Modules/processors\"")
system("$$COPY_DIR \"$$BUILD_DIR/Modules/rasterProviders\" \"$$RUN_DIR/Modules/rasterProviders\"")

# Copy config files and dynamic libraries
system("$$COPY_DIR \"$$CONFIG_DIR/Setup\" \"$$RUN_DIR\"")
system("$$COPY_DIR \"$$CONFIG_DIR/xml\" \"$$RUN_DIR/xml\"")
system("$$COPY_DIR \"$$BUILD_DIR/Libraries/dll\" \"$$RUN_DIR\"")
}

unix{
# Copy Modules in Run directory
system("$$COPY_DIR $$BUILD_DIR/Modules/imageProviders $$RUN_DIR/Modules")
system("$$COPY_DIR $$BUILD_DIR/Modules/processors $$RUN_DIR/Modules")
system("$$COPY_DIR $$BUILD_DIR/Modules/rasterProviders $$RUN_DIR/Modules")

# Copy config files and dynamic libraries
system("$$COPY_DIR $$CONFIG_DIR/Setup/* $$RUN_DIR/")
system("$$COPY_DIR $$CONFIG_DIR/xml $$RUN_DIR")
system("$$COPY_DIR $$BUILD_DIR/Libraries/dll/* $$RUN_DIR")
}
