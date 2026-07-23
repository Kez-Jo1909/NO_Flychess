cmake_minimum_required(VERSION 3.10)
project(NO_Flychess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 全局 UTF-8 编译选项，解决 MSVC C4819 警告
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 设置 Qt 路径
set(CMAKE_PREFIX_PATH "C:/Qt/5.15.19/msvc2019_64")

# =========================
# 1. 全局设置 GUI 程序（不弹控制台）
# =========================
if(WIN32)
    # 所有可执行程序默认使用 Windows 子系统
    set(CMAKE_WIN32_EXECUTABLE TRUE)
endif()

# =========================
# 2. 子目录（库和可执行文件）
# =========================
add_subdirectory(flychess_game)
add_subdirectory(third_party/ixwebsocket)
add_subdirectory(server)
add_subdirectory(client)

# =========================
# 3. 安装打包设置
# =========================
include(GNUInstallDirs)

# 假设客户端目标名是 Flychess_QT_Client
# 如果实际可执行文件名不同，请替换
install(TARGETS Flychess_QT_Client
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}) 

# 安装依赖的 DLL（手动列或用 Qt 工具自动处理）
if(WIN32)
    # 假设 Qt 的 DLL 都在 bin 目录下
    file(GLOB QT_DLLS "${CMAKE_PREFIX_PATH}/bin/Qt5*.dll")
    install(FILES ${QT_DLLS} DESTINATION ${CMAKE_INSTALL_BINDIR})

    # 拷贝 Qt 插件（例如 imageformats、platforms）
    install(DIRECTORY "${CMAKE_PREFIX_PATH}/plugins/platforms"
            DESTINATION ${CMAKE_INSTALL_BINDIR}/..)

    install(DIRECTORY "${CMAKE_PREFIX_PATH}/plugins/imageformats"
            DESTINATION ${CMAKE_INSTALL_BINDIR}/..)
endif()

# =========================
# 4. 安装 config 目录（完整打包）
# =========================
set(CONFIG_DIR ${CMAKE_SOURCE_DIR}/config)

if(EXISTS ${CONFIG_DIR})
    # 复制整个 config 目录到 bin 同级
    install(DIRECTORY ${CONFIG_DIR}
            DESTINATION ${CMAKE_INSTALL_BINDIR}/..)
else()
    message(WARNING "配置目录 ${CONFIG_DIR} 未找到，安装时将跳过。")
endif()
