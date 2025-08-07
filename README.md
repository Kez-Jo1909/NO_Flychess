# README

![version](https://img.shields.io/badge/version-0.1.0-black)

## 依赖项

- Emscripten

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
chmod +x build.sh
./build.sh
cd frontend
npx http-server
```

## TODO

- [x] JS图形绘制封装
- [ ] 将grid类重构为基类，派生出其余格子
- [x] preGoal区域移动，分前后两类
- [x] bridge区域

## 组件逻辑

```mathematica
        ┌─────────────┐
        │   index.html│ ← 网页结构和入口（HTML）
        └─────┬───────┘
              │ 引入
        ┌─────▼──────────────┐
        │ game.js(由emcc生成) │ ← JavaScript接口桥梁
        └─────┬──────────────┘
              │ 加载 + 调用
        ┌─────▼─────────────┐
        │    game.wasm      │ ← WebAssembly模块（C++编译结果）
        └─────┬─────────────┘
              │ 调用
        ┌─────▼─────────────┐
        │  C++游戏逻辑       │ ← 高性能核心代码（棋盘、规则等）
        └───────────────────┘

```

| 技术           | 作用                                                    | 示例代码                  |
| -------------- | ------------------------------------------------------- | ------------------------- |
| **C++**        | 实现高性能、核心逻辑，如飞行棋规则、锦囊牌算法          | `roll_dice_and_move()`    |
| **WASM**       | WebAssembly，C++ 编译后的格式，浏览器可直接运行的字节码 | `game.wasm`               |
| **JavaScript** | 作为“桥梁”调用 WASM 中的 C++ 函数，并处理界面和交互     | `Module.cwrap(...)`       |
| **HTML**       | 网页结构，提供按钮、文本、容器等                        | `<button>掷骰子</button>` |

## EMSCRIPTEN

| 方法                   | 作用                        | 示例                             |
| ---------------------- | --------------------------- | -------------------------------- |
| `EMSCRIPTEN_KEEPALIVE` | 保留 C++ 函数供 JS 调用     | `EMSCRIPTEN_KEEPALIVE int f()`   |
| `Module.cwrap()`       | 从 JS 调用 C++ 函数         | `Module.cwrap("roll_dice", ...)` |
| `Module.ccall()`       | 直接调用（比 cwrap 更底层） | `Module.ccall("get_score", ...)` |

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

| Color     | Utils内定义 | Js内定义 | 具体颜色 |
| --------- | ----------- | -------- | -------- |
| UNDEFINED | -1          | 0        | gray     |
| RED       | 0           | 1        | red      |
| BLUE      | 1           | 2        | #6666ff  |
| GREEN     | 2           | 3        | #66ff66  |
| YELLOW    | 3           | 4        | yellow   |

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
cmake --install . build --config Release
& "C:\Qt\5.15.19\msvc2019_64\bin\windeployqt.exe" .\Flychess_QT_Client.exe
```