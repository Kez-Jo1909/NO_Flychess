project(flychess_game)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

# command.cpp 已经通过 GLOB_RECURSE 自动包含
# 如果 GLOB 没有捡到新文件，手动加：
# list(APPEND SOURCES ${PROJECT_SOURCE_DIR}/src/command.cpp)

add_library(flychess STATIC ${SOURCES})

target_include_directories(flychess PUBLIC ${PROJECT_SOURCE_DIR}/include)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(flychess PUBLIC nlohmann_json::nlohmann_json)