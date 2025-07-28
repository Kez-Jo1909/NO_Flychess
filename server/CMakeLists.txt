cmake_minimum_required(VERSION 3.10)
project(FlychessServer)

set(CMAKE_CXX_STANDARD 17)

# 添加头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 递归查找所有 .cpp 源文件
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# 添加可执行文件
add_executable(server ${SOURCES})

target_link_libraries(server PRIVATE ixwebsocket flychess)
