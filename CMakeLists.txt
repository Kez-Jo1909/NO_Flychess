cmake_minimum_required(VERSION 3.10)
project(NO_Flychess)

# 其他子目录
add_subdirectory(flychess_game)

# 先添加 ixwebsocket，方便 server 用
add_subdirectory(third_party/ixwebsocket)

add_subdirectory(server)

add_subdirectory(client)
