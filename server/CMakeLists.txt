cmake_minimum_required(VERSION 3.10)
project(FlychessServer)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# 生成静态库
add_library(flychess_server STATIC ${SOURCES})

# 显式暴露 server/include 目录给外部使用
target_include_directories(flychess_server
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# 链接依赖库
target_link_libraries(flychess_server
    PUBLIC
        ixwebsocket
        flychess
)
