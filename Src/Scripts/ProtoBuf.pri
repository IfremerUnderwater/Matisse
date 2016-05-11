win32 {
    INCLUDEPATH *= $$(PROTOBUF_DIR)/include

    CONFIG(debug, debug|release) {
        LIBS *= -L$$(PROTOBUF_DIR)/lib/Debug
    }
    else {
        LIBS *= -L$$(PROTOBUF_DIR)/lib/Release
    }

}
win32{
LIBS += -llibprotobuf
}
else{
LIBS += -lprotobuf
}
if (!isEmpty(PROTOS)){

message("Generating protocol buffer classes from .proto files.")

message($$LIBS)

protobuf_decl.name = protobuf headers
protobuf_decl.input = PROTOS
protobuf_decl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.h
win32{
protobuf_decl.commands = protoc.exe --cpp_out=${QMAKE_FILE_IN_PATH} --proto_path=${QMAKE_FILE_IN_PATH} ${QMAKE_FILE_NAME}
}
else{
protobuf_decl.commands = protoc --cpp_out=${QMAKE_FILE_IN_PATH} --proto_path=${QMAKE_FILE_IN_PATH} ${QMAKE_FILE_NAME}
}
protobuf_decl.variable_out = HEADERS
QMAKE_EXTRA_COMPILERS += protobuf_decl

protobuf_impl.name = protobuf sources
protobuf_impl.input = PROTOS
protobuf_impl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.cc
protobuf_impl.depends = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.pb.h
protobuf_impl.commands = $$escape_expand(\\n)
protobuf_impl.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += protobuf_impl

}
