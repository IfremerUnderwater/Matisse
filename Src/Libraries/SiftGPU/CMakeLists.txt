if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_definitions("-DSIFTGPU_NO_DEVIL")

set(SIFT_GPU_SOURCE_FILES
    FrameBufferObject.cpp
    FrameBufferObject.h
    GlobalUtil.cpp
    GlobalUtil.h
    GLTexImage.cpp
    GLTexImage.h
    ProgramGLSL.cpp
    ProgramGLSL.h
    ProgramGPU.h
    PyramidGL.cpp
    PyramidGL.h
    ShaderMan.cpp
    ShaderMan.h
    SiftGPU.cpp
    SiftGPU.h
    SiftMatch.cpp
    SiftMatch.h
    SiftPyramid.cpp
    SiftPyramid.h
)


add_library(SiftGpu ${SIFT_GPU_SOURCE_FILES})

target_link_libraries(SiftGpu
    ${SIFT_GPU_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    ${OPENGL_glu_LIBRARY}
)

# state that anybody linking to us needs to include the current source dir
target_include_directories(SiftGpu
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
