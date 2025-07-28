cmake_minimum_required(VERSION 3.10)
project(flychess_game)

# 使用 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 用的是Emscripten，设置输出格式
set(CMAKE_EXECUTABLE_SUFFIX ".js")

# 包含头文件目录
include_directories(${PROJECT_SOURCE_DIR}/../flychess_game/include)

# 添加所有源码文件
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/../flychess_game/src/*.cpp
)

# 设置输出文件名为 game（会输出 game.js / game.wasm）
add_executable(game ${SOURCES})

# Emscripten 编译相关 flags（生成 .js 和 .wasm）
# set_target_properties(game PROPERTIES
#     LINK_FLAGS "--bind -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME=\"FlyChessModule\" -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS='[\"cwrap\", \"ccall\", \"HEAP32\"]'"
# )
set_target_properties(game PROPERTIES
    LINK_FLAGS "--bind -s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME=FlyChessModule -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS=[\"cwrap\",\"ccall\",\"HEAP32\"] -O3 -DNDEBUG"
)




