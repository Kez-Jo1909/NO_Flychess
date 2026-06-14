# NO_Flychess

![version](https://img.shields.io/badge/version-0.4.0-black)

糅杂了卡牌系统的局域网飞行棋游戏。高三时期自制卡牌飞行棋桌游的电子化实现。

Electron 客户端 + C++ 游戏引擎 + WebSocket 通信，支持局域网联机。

## 架构

```
Electron 客户端 (JS/HTML)
├── renderer/          # 前端 UI — 棋盘、卡牌、骰子、聊天
├── main.js            # Electron 主进程 — IPC、启动/停止服务端
└── server-bin/        # 内嵌的 C++ 服务端 + 配置文件

C++ 服务端 (server/)
├── flychess           # 游戏核心引擎（静态库）
├── flychess_server    # WebSocket 服务端 — 房间管理、消息分发、卡牌结算
├── ixwebsocket        # WebSocket 通信库
└── nlohmann_json      # JSON 解析
```

```
Electron UI ──WS──▶ Server ──规则引擎──▶ flychess_game
    (渲染)         (房间/消息)         (数据+逻辑)
```

## 项目结构

```
NO_Flychess/
├── electron-client/       # Electron 桌面客户端
│   ├── main.js            # 主进程（IPC + 本地服务端启停）
│   ├── preload.js         # 安全的 IPC 桥接
│   ├── renderer/          # 渲染进程
│   │   ├── index.html     # 单页应用
│   │   ├── app.js         # 路由 & 全局状态
│   │   ├── pages/         # home.js, lobby.js, game.js
│   │   ├── components/    # board.js, card.js, dice.js, chat.js
│   │   └── network/       # ws.js (WebSocket 客户端)
│   ├── server-bin/        # 打包时复制服务端 exe（gitignore）
│   └── assets/            # 卡牌图片等资源
├── flychess_game/         # C++ 游戏核心引擎（静态库）
│   ├── include/           # game.h, player.h, map.h, card.h, command.h, utils.h
│   └── src/               # game.cpp, player.cpp, map.cpp, card.cpp, command.cpp
├── server/                # C++ WebSocket 服务端
│   ├── include/           # server.h
│   └── src/               # server.cpp, main.cpp
├── config/                # 游戏配置
│   ├── card.json          # 卡牌数据（DSL 驱动）
│   ├── game_map.json      # 棋盘地图
│   └── version.json       # 版本信息
├── scripts/               # 构建 & 发布脚本
│   └── release.bat        # 一键 Release 打包
├── frontend/              # WASM 网页版（已搁置）
└── third_party/           # ixwebsocket (git submodule)
```

## 依赖

| 依赖 | 用途 |
|------|------|
| Node.js 18+ / npm | Electron 运行 & 打包 |
| CMake 3.14+ | C++ 构建系统 |
| MSVC 2019+ (Windows) | C++ 编译器 |
| nlohmann_json | JSON 解析 (vcpkg) |
| ixwebsocket | WebSocket (git submodule) |

## 编译 & 运行（开发）

```bash
# 1. 拉取子模块
git submodule update --init --recursive

# 2. C++ 服务端
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="D:/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Release
# CMake 自动把 config/ 复制到 build/server/config/

# 3. Electron 客户端
cd electron-client
npm install
npm start          # npx electron . ，开发模式
npm run dev        # 开发模式 + 打开 DevTools
```

## 一键发布

```bat
scripts\release.bat 0.5.0
```

自动完成：编译 C++ 服务端 → 复制到打包目录 → 更新版本号 → 打包 Electron NSIS 安装器。

产物在 `electron-client/dist/`。

## 玩法

N 人飞行棋 + 卡牌系统。掷骰子走棋子，掷出 5 获得一张卡牌，掷出 6 可以再掷一次。

### 卡牌系统

卡牌全部由 `config/card.json` 驱动，使用 DSL 指令集描述效果。扩展新卡牌只需添加 JSON 条目。

| ID | 名称 | 时机 | 效果 |
|----|------|------|------|
| 0 | 6 | 掷骰前 | 使自己本回合骰子变为 6 |
| 1 | 极端天气 | 任意 | 所有处于待飞区的飞机返回重生点 |

### 卡牌 DSL 指令

| 指令 | 说明 |
|------|------|
| `set_dice` | 设置骰子点数 |
| `send_to_start` | 将指定棋子送回重生点 |
| `send_all_pre_to_start` | 将所有待飞区棋子送回重生点 |
| `move_piece` | 移动棋子 |
| `fly_piece` | 飞行棋子 |
| `custom` | 自定义 C++ 指令 |

### 卡牌时机

| 时机 | 说明 |
|------|------|
| `ANYTIME` (0) | 任意时刻 |
| `BEFORE_ROLL` (2) | 掷骰子前（ROLLING 阶段） |
| `BEFORE_MOVE` (3) | 移动棋子前（SELECTING 阶段） |
| `AFTER_MOVE` (4) | 移动棋子后（CARDING 阶段） |

### 规则要点

- 掷出 6：可以起飞一个棋子 / 移动后多一个回合
- 踩到别人的棋子：踢回起点
- 棋子必须精确到达终点
- 手牌上限 5 张，超限需弃牌
- 只有一个可动棋子时自动移动

## 配置

- `config/card.json` — 卡牌列表（id, name, description, function_time, effects[]）
- `config/game_map.json` — 地图格子类型、位置
- `config/version.json` — 版本号

## 卡牌扩展示例

```json
{
    "id": 2,
    "name": "突进",
    "description": "选择一个棋子向前移动 3 步",
    "function_time": 3,
    "image_path": "../assets/cards/rush.jpg",
    "target_selection": 1,
    "effects": [
        {"op": "move_piece", "player": "$caster", "piece_id": "$selected_piece", "steps": 3}
    ]
}
```
