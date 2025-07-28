project(flychess_game)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

add_library(flychess STATIC ${SOURCES})

target_include_directories(flychess PUBLIC ${PROJECT_SOURCE_DIR}/include)

