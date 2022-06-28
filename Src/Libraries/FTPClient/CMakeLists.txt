set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(${CURL_INCLUDE_DIRS})

add_library(FTPClient CurlHandle.cpp FTPClient.cpp threadable_ftpclient.cpp qftpclient.cpp)

target_link_libraries(FTPClient Qt5::Core NetworkTools)

# state that anybody linking to us needs to include the current source dir
target_include_directories(FTPClient
          INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
          )
