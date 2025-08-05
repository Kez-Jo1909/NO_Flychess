cmake_minimum_required(VERSION 3.10)
project(NO_Flychess)

set(CMAKE_CXX_STANDARD 17)

# 针对 Windows MSVC 强制 UTF-8 编译
if (MSVC)
    add_compile_options(/utf-8)
endif()

# 针对 MinGW 或 Clang on Windows（可选）
if (WIN32 AND NOT MSVC)
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# 其他子目录
add_subdirectory(flychess_game)
add_subdirectory(third_party/ixwebsocket)
add_subdirectory(server)
add_subdirectory(client)
