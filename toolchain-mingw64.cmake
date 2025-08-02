 # 指定目标系统为 Windows
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# 指定交叉编译器
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# MinGW 的 Windows 资源编译器
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# 指定工具链根路径，避免 Linux 库干扰
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# 查找规则：只在交叉环境找库和头文件
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
