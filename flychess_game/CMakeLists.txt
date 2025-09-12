project(flychess_game)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_library(flychess STATIC ${SOURCES})

target_include_directories(flychess PUBLIC ${PROJECT_SOURCE_DIR}/include)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(flychess PUBLIC nlohmann_json::nlohmann_json)