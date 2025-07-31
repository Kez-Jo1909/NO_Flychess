cmake_minimum_required(VERSION 3.10)
project(FlychessServer)

set(CMAKE_CXX_STANDARD 17)

# 启用自动 MOC/UIC/RCC（处理 Q_OBJECT、.ui、.qrc）
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找 Qt5 库
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)

# 查找源码文件（包含头文件，让 AUTOMOC 能扫描 Q_OBJECT）
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/include/*.h
)

# 构建静态库
add_library(flychess_server STATIC ${SOURCES})

# 公开 include 目录
target_include_directories(flychess_server
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# 链接依赖库
target_link_libraries(flychess_server
    PUBLIC
        ixwebsocket
        flychess
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
)
