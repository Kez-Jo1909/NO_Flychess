cmake_minimum_required(VERSION 3.10)
project(FlychessServer)

set(CMAKE_CXX_STANDARD 17)

find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/include/*.h
)

# 从静态库中排除 main.cpp
list(FILTER SOURCES EXCLUDE REGEX ".*/main\\.cpp$")

# 服务端静态库（游戏逻辑 + 网络处理）
add_library(flychess_server STATIC ${SOURCES})

target_include_directories(flychess_server
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(flychess_server
    PUBLIC
        ixwebsocket
        flychess
        nlohmann_json::nlohmann_json
)

# 服务端可执行文件
add_executable(FlychessServer src/main.cpp)
target_link_libraries(FlychessServer
    PRIVATE
        flychess_server
        ixwebsocket
        flychess
        nlohmann_json::nlohmann_json
)
