# README

![version](https://img.shields.io/badge/version-0.0.1-black)

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