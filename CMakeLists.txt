cmake_minimum_required(VERSION 3.10)
project(NO_Flychess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 全局 UTF-8 编译选项，解决 MSVC C4819 警告
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Qt 已卸载，客户端迁移至 Electron
# set(CMAKE_PREFIX_PATH "C:/Qt/5.15.19/msvc2019_64")

# 全局设置（Qt 客户端已不在此构建系统内）
# if(WIN32)
#     set(CMAKE_WIN32_EXECUTABLE TRUE)
# endif()

# 2. 子目录（库和可执行文件）
add_subdirectory(flychess_game)
add_subdirectory(third_party/ixwebsocket)
add_subdirectory(server)
# add_subdirectory(client)  # Qt 已卸载，Electron 客户端替代

# 3. 安装打包设置
include(GNUInstallDirs)

# Qt 客户端已替换为 Electron，不再需要 Qt DLL 打包
# install(TARGETS Flychess_QT_Client
#         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# 安装 config 目录（完整打包）
set(CONFIG_DIR ${CMAKE_SOURCE_DIR}/config)

if(EXISTS ${CONFIG_DIR})
    install(DIRECTORY ${CONFIG_DIR}
            DESTINATION ${CMAKE_INSTALL_BINDIR}/..)
else()
    message(WARNING "配置目录 ${CONFIG_DIR} 未找到，安装时将跳过。")
endif()
