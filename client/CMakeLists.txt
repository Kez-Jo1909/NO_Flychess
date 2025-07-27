cmake_minimum_required(VERSION 3.14)
project(Flychess_QT_Client)

set(CMAKE_CXX_STANDARD 17)

# 自动处理 Qt MOC/UIC/RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找 Qt5 Widgets 模块
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# 添加头文件目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 查找所有源文件
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# 定义可执行目标
add_executable(Flychess_QT_Client ${SOURCES})

# 链接 Qt5 库
target_link_libraries(Flychess_QT_Client Qt5::Widgets)
