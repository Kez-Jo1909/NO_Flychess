# README

![version](https://img.shields.io/badge/version-0.2.3-black)

## 依赖项

- nlomann json

  `sudo apt install nlohmann-json3-dev`

- Qt5

- ixWebsocket

  ```bash
  #集成在third_party中
  git submodule update --init --recursive
  ```

- Emscripten（已丢弃，如需编译wasm供frontend使用需要）

```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

若当前zsh主题设置为**Powerlevel10k**，可能导致`[WARNING]: Console output during zsh initialization detected`，此时将~/.zshrc中改为

```bash
export EMSDK_QUIET=1
source /home/kezjo/emsdk/emsdk_env.sh > /dev/null
```

## 编译方式

```bash
chmod ./build.sh
./buils.sh
```

## TODO

- [x] JS图形绘制封装
- [x] preGoal区域移动，分前后两类
- [x] bridge区域
- [x] map重构

## MapGrid备注

| 类型                         | vector引导         | ID     | COLOR     |
| ---------------------------- | ------------------ | ------ | --------- |
| HOME                         | 0-15               | -1     | 4种       |
| NORMAL                       | 16-71(终点前52-71) | 1-57中 | 4种       |
| TURN&BRIDGE                  | 72-87              | 1-52中 | 4种       |
| GOAL                         | 88-91              | 58     | 4种       |
| START                        | 92-95              | 0      | UNDEFINED |
| NORMAL(已完成的得找个地方放) | 96                 | -2     | UNDEFINED |

> [!WARNING]
>
> 终点前区域，vector内是先4个53，4个54...



## Color结构体定义

| Color     | Utils内定义 | Js内定义 | 具体颜色    |
| --------- | ----------- | -------- | ----------- |
| UNDEFINED | -1          | 0        | 200,200,200 |
| RED       | 0           | 1        | 255,0,0     |
| BLUE      | 1           | 2        | 0,0,255     |
| GREEN     | 2           | 3        | 0,255,0     |
| YELLOW    | 3           | 4        | 255,255,0   |

## Github Pages

安装 `gh-pages` 工具（只做一次）：

```bash
npm install -g gh-pages
```

进入项目根目录，发布 frontend 内容：

```bash
gh-pages -d frontend
```

它会自动创建 `gh-pages` 分支并推送 frontend 文件夹的内容

## Windows编译

```bash
git submodule update --init --recursive
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="D:/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_PREFIX_PATH="C:/Qt/5.15.19/msvc2019_64" -DCMAKE_INSTALL_PREFIX="%cd%/dist"
cmake --build . --config Release
cmake --install . --config Release
& "C:\Qt\5.15.19\msvc2019_64\bin\windeployqt.exe" .\Flychess_QT_Client.exe
```