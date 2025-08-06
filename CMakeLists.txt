cmake_minimum_required(VERSION 3.10)
project(NO_Flychess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 全局 UTF-8 编译选项，解决C4819警告
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 设置 Qt 路径，方便子目录 find_package
set(CMAKE_PREFIX_PATH "C:/Qt/5.15.19/msvc2019_64")

# 添加子目录，顺序要保证依赖关系
add_subdirectory(flychess_game)

add_subdirectory(third_party/ixwebsocket)

add_subdirectory(server)

add_subdirectory(client)
