# Workaround to be removed in qt5 with qmake.conf and shadowed function --
SOURCE_DIR=$$PWD/..
SERVER_SOURCE_DIR=$${SOURCE_DIR}/MatisseServer
MATISSE_TOOLS_SOURCE_DIR=$${SOURCE_DIR}/Libraries/MatisseTools
CONFIG_DIR=$${SOURCE_DIR}/../Config
message("CONFIG_DIR = $$CONFIG_DIR")
BUILD_DIR=$${SOURCE_DIR}/../Build/Release
RUN_DIR=$${SOURCE_DIR}/../Run/Release

# ------------------------------------------------------------------------

message("RUN_DIR = $$RUN_DIR")

win32{
    DEL_FILE      = del
    MKDIR         = mkdir
    COPY          = copy /y
    COPY_DIR      = xcopy /s /q /y /i
    DEL_DIR       = rmdir
    MOVE          = move
    CHDIR         = cd
}

unix{
    DEL_FILE      = rm -f
    MKDIR         = mkdir
    COPY          = cp
    COPY_DIR      = cp -rf
    DEL_DIR       = rm -rf
    MOVE          = mv
    CHDIR         = cd
}



# Create directories
system("$$MKDIR \"$$RUN_DIR/Modules\"")
system("$$MKDIR \"$$RUN_DIR/Modules/imageProviders\"")
system("$$MKDIR \"$$RUN_DIR/Modules/processors\"")
system("$$MKDIR \"$$RUN_DIR/Modules/rasterProviders\"")
system("$$MKDIR \"$$RUN_DIR/Libraries\"")
system("$$MKDIR \"$$RUN_DIR/help\"")
system("$$MKDIR \"$$RUN_DIR/lnf\"")
system("$$MKDIR \"$$RUN_DIR/xml\"")
system("$$MKDIR \"$$RUN_DIR/i18n\"")
system("$$MKDIR \"$$RUN_DIR/schemas\"")
system("$$MKDIR \"$$RUN_DIR/platform\"")

win32{
# Copy Modules in Run directory
system("$$COPY_DIR \"$$BUILD_DIR/Modules/imageProviders\" \"$$RUN_DIR/Modules/imageProviders\"")
system("$$COPY_DIR \"$$BUILD_DIR/Modules/processors\" \"$$RUN_DIR/Modules/processors\"")
system("$$COPY_DIR \"$$BUILD_DIR/Modules/rasterProviders\" \"$$RUN_DIR/Modules/rasterProviders\"")

# Copy config files and dynamic libraries
system("$$COPY_DIR \"$$CONFIG_DIR/Setup\" \"$$RUN_DIR\"")
system("$$COPY_DIR \"$$CONFIG_DIR/lnf\" \"$$RUN_DIR/lnf\"")
system("$$COPY_DIR \"$$CONFIG_DIR/help\" \"$$RUN_DIR/help\"")
system("$$COPY_DIR \"$$CONFIG_DIR/config\" \"$$RUN_DIR/config\"")
system("$$COPY_DIR \"$$CONFIG_DIR/schemas\" \"$$RUN_DIR/schemas\"")
system("$$COPY_DIR \"$$CONFIG_DIR/xml\" \"$$RUN_DIR/xml\"")
system("$$COPY_DIR \"$$BUILD_DIR/Libraries/dll\" \"$$RUN_DIR/Libraries\"")

# Copy linguist files
#system("echo $$CHDIR \"$$SERVER_SOURCE_DIR\"")
#system("$$CHDIR \"$$SERVER_SOURCE_DIR\"")
system("echo $$COPY \"..\\MatisseServer\\*.qm\" \"$$RUN_DIR/i18n\"")
system("$$COPY \"..\\MatisseServer\\*.qm\" \"$$RUN_DIR/i18n\"")
system("$$COPY \"..\\Libraries\\MatisseTools\\*.qm\" \"$$RUN_DIR/i18n\"")
}

unix{
# Copy Modules in Run directory
system("$$COPY_DIR $$BUILD_DIR/Modules/imageProviders $$RUN_DIR/Modules")
system("$$COPY_DIR $$BUILD_DIR/Modules/processors $$RUN_DIR/Modules")
system("$$COPY_DIR $$BUILD_DIR/Modules/rasterProviders $$RUN_DIR/Modules")

# Copy config files and dynamic libraries
#system("$$COPY_DIR $$CONFIG_DIR/Setup/* $$RUN_DIR/")
#system("$$COPY_DIR $$CONFIG_DIR/xml $$RUN_DIR")
system("$$COPY_DIR $$CONFIG_DIR/lnf $$RUN_DIR")
system("$$COPY_DIR $$CONFIG_DIR/help $$RUN_DIR")
system("$$COPY_DIR $$CONFIG_DIR/config $$RUN_DIR")
system("$$COPY_DIR $$BUILD_DIR/Libraries/dll/* $$RUN_DIR/Libraries")
system("$$COPY_DIR $$CONFIG_DIR/schemas $$RUN_DIR")

# Copy linguist files
#system("echo $$CHDIR \"$$SERVER_SOURCE_DIR\"")
#system("$$CHDIR \"$$SERVER_SOURCE_DIR\"")
system("$$COPY_DIR $$SERVER_SOURCE_DIR/*.qm $$RUN_DIR/i18n")
system("$$COPY_DIR $$MATISSE_TOOLS_SOURCE_DIR/*.qm $$RUN_DIR/i18n")
}
