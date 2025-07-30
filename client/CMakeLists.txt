cmake_minimum_required(VERSION 3.14)
project(Flychess_QT_Client)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

add_executable(Flychess_QT_Client ${SOURCES} ${HEADERS})

# 只包含 client 的头文件
target_include_directories(Flychess_QT_Client
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(Flychess_QT_Client
    PRIVATE
        flychess
        flychess_server
        Qt5::Widgets
)
