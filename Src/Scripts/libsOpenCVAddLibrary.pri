#isEmpty(OPENCV_DIR){
#    error("OPENCV_DIR is not declared (check .qmake.conf)")
#}

#!exists($$OPENCV_DIR) {
#  error("OPENCV_DIR does not exists, $$OPENCV_INSTALL_PREFIX ")
#}

defineTest(LibsAddLibrary) {
   
  INCLUDEPATH *= $${OPENCV_DIR}/../../include

  LIBS *=  -L$$(OPENCV_DIR)/lib
  message("LIBS " = $$LIBS)

  LIBS *= -lopencv_$${1}

  message("Add lib = -lopencv_$${1}")

  export(CONFIG)
  export(INCLUDEPATH)
  export(LIBS)
}
