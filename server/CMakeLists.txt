cmake_minimum_required(VERSION 3.10)
project(FlychessServer)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)

find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/include/*.h
)

add_library(flychess_server STATIC ${SOURCES})

target_include_directories(flychess_server
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(flychess_server
    PUBLIC
        ixwebsocket
        flychess
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
        nlohmann_json::nlohmann_json
)
