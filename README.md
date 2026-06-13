# NO_Flychess

![version](https://img.shields.io/badge/version-0.3.0-black)

糅杂了卡牌系统的局域网飞行棋游戏。高三时期自制卡牌飞行棋桌游的电子化实现。

基于 Qt5 客户端，ixwebsocket 通信，支持局域网联机。游戏引擎独立为静态库，可编译到 WASM 供网页端使用（网页端已搁置）。

## 架构

```
Flychess_QT_Client.exe
├── flychess        (静态库) — 游戏核心引擎
├── flychess_server (静态库) — WebSocket 服务端
├── ixwebsocket     (静态库) — WebSocket 通信
├── nlohmann_json   (header-only) — JSON 解析
└── Qt5             (动态库) — GUI 框架
```

游戏引擎、服务端、ixwebsocket 全部静态链接进 exe。Qt5 动态链接，发布时需要带 Qt5 DLL 和插件目录。

```
client/   ──UI──▶  flychess_game/  ◀──server──▶  ixwebsocket  ──网络──▶  其他玩家
(界面)             (数据+规则)       (房间管理)    (通信)
```

客户端只管 UI 和渲染，所有游戏逻辑在 `flychess_game` 里，服务端负责房间管理、消息转发和状态同步。

## 项目结构

```
NO_Flychess/
├── flychess_game/       # 游戏核心引擎（静态库）
│   ├── include/         # game.h, map.h, player.h, card.h, utils.h
│   └── src/             # game.cpp, map.cpp, player.cpp, card.cpp, utils.cpp
├── server/              # WebSocket 服务端（静态库）
│   ├── include/         # server.h, client.h
│   └── src/             # server.cpp, client.cpp, main.cpp
├── client/              # Qt5 桌面客户端
│   ├── include/         # MainWindow.h
│   └── src/             # MainWindow.cpp, MainWindow.ui, main.cpp
├── config/              # 游戏配置文件
│   ├── card.json        # 卡牌数据
│   ├── game_map.json    # 地图数据
│   └── version.json     # 版本信息
├── assets/              # 图片资源
│   └── cards/           # 卡牌图片
├── frontend/            # 网页前端（已搁置）
├── wasm/                # Emscripten/WASM 编译入口
├── third_party/         # 第三方依赖（git submodule）
│   └── ixwebsocket/
└── build.sh             # Linux 编译脚本
```

## 依赖

| 依赖 | 用途 | 备注 |
|------|------|------|
| Qt 5.15+ | GUI 框架 | Core, Widgets, Network 模块 |
| nlohmann_json | JSON 解析 | header-only，vcpkg 安装 |
| ixwebsocket | WebSocket | git submodule，静态链接 |
| CMake 3.14+ | 构建系统 | |
| MSVC 2019+ / GCC | 编译器 | C++17 |

Emscripten 用于编译 WASM 目标（可选，网页端已搁置）。

## 编译

### Windows (MSVC)

```bash
# 1. 拉取子模块
git submodule update --init --recursive

# 2. 配置（vcpkg 提供 nlohmann_json）
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 \
    -DCMAKE_TOOLCHAIN_FILE="D:/vcpkg/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET=x64-windows \
    -DCMAKE_PREFIX_PATH="C:/Qt/5.15.19/msvc2019_64" \
    -DCMAKE_INSTALL_PREFIX="%cd%/dist"

# 3. 编译 & 安装
cmake --build . --config Release
cmake --install . --config Release

# 4. 自动拉取 Qt 动态库
& "C:\Qt\5.15.19\msvc2019_64\bin\windeployqt.exe" .\Flychess_QT_Client.exe
```

> [!NOTE]
> windeployqt 只拉 Qt 相关 DLL。如果缺少 `zlib1.dll`（ixwebsocket 的 TLS 依赖），需要手动拷贝（已附带在文件夹中）。

### Linux

```bash
# 安装系统依赖
sudo apt install nlohmann-json3-dev qt5-default

# 编译
chmod +x build.sh
./build.sh
```

`build.sh` 内容：

```bash
set -e
mkdir -p build
cd build
cmake ..
make -j16
cd ..
echo "✅ 编译全部完成"
```

### WASM（已搁置）

```bash
./wasm_build.sh
```

产物拷贝到 `frontend/` 目录，用任意 HTTP 服务器托管即可。

## 运行

### 服务端

```bash
cd build/server
./server
```

服务端负责房间管理、消息转发。客户端连接后可以创建/加入房间。

### 客户端

```bash
cd build/client
./Flychess_QT_Client
```

或在 Windows 上直接双击 exe。

### 网页端（已搁置）

用任意 HTTP 服务器托管 `frontend/` 目录，浏览器打开 `index.html`。

## 玩法

四人飞行棋 + 卡牌系统。每回合抽一张卡，出牌产生效果，然后掷骰子走棋子。

### 当前卡牌

| ID | 名称 | 效果 |
|----|------|------|
| 0 | 6 | 使自己投出一个 6 |
| 1 | 极端天气 | 所有处于待飞区的飞机返回重生点 |

卡牌在 `config/card.json` 中配置，扩展新卡牌只需添加 JSON 条目 + 实现对应的 `card_function`。

### 规则要点

- 掷出 6 可以再掷一次
- 踩到别人棋子踢回起点
- 棋子必须精确到达终点（终点前区域有独立格子）

## 配置

所有游戏参数通过 `config/` 下的 JSON 文件配置，不硬编码。

- `game_map.json` — 地图格子类型、位置、颜色
- `card.json` — 卡牌列表（id, name, description, function_time, image_path, target_selection）
- `version.json` — 版本号和更新说明

## MapGrid 备注

| 类型 | vector 引导 | ID | COLOR |
|------|-------------|----|-------|
| HOME | 0-15 | -1 | 4 种 |
| NORMAL | 16-71（终点前 52-71） | 1-57 中 | 4 种 |
| TURN & BRIDGE | 72-87 | 1-52 中 | 4 种 |
| GOAL | 88-91 | 58 | 4 种 |
| START | 92-95 | 0 | UNDEFINED |
| NORMAL（已完成放这里） | 96 | -2 | UNDEFINED |

> [!WARNING]
> 终点前区域，vector 内是先 4 个 53，4 个 54...

## Color 定义

| Color | Utils 内 | JS 内 | RGB |
|-------|----------|-------|-----|
| UNDEFINED | -1 | 0 | 200, 200, 200 |
| RED | 0 | 1 | 255, 0, 0 |
| BLUE | 1 | 2 | 0, 0, 255 |
| GREEN | 2 | 3 | 0, 255, 0 |
| YELLOW | 3 | 4 | 255, 255, 0 |

## TODO

- [x] JS 图形绘制封装
- [x] preGoal 区域移动（前后两类）
- [x] bridge 区域
- [x] map 重构
- [x] 卡牌系统（抽卡、出牌、动画、弃牌）
- [ ] 右键点击卡牌弹出提示
- [ ] 卡牌动画偶发闪退
- [ ] 联网自动更新
- [ ] 局域网房间列表（目前需手动输入 IP）
- [ ] AI 玩家
- [ ] 更多卡牌

## Github Pages

```bash
npm install -g gh-pages
gh-pages -d frontend
```

自动创建 `gh-pages` 分支并推送 frontend 内容。

## 交叉编译 (MinGW)

项目提供了 `toolchain-mingw64.cmake`，在 Linux 上交叉编译 Windows 目标：

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw64.cmake
```
