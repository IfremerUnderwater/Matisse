set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${CURL_INCLUDE_DIRS})

file(GLOB_RECURSE source_files ./*)
add_library(FTPClient CurlHandle.cpp FTPClient.cpp)

# state that anybody linking to us needs to include the current source dir
target_include_directories(FTPClient
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
