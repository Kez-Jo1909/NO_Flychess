#!/bin/bash
# ============================================================
# NO_Flychess Release 打包脚本
# 用法: bash scripts/release.sh [版本号]
# 示例: bash scripts/release.sh 0.5.0
# ============================================================
set -e

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT/build"
SERVER_BIN_DIR="$ROOT/electron-client/server-bin"
ELECTRON_DIR="$ROOT/electron-client"

# 版本号（从参数或 package.json 读取）
VERSION="${1:-$(node -p "require('$ELECTRON_DIR/package.json').version" 2>/dev/null || echo '0.0.0')}"
echo "========================================"
echo " NO_Flychess Release 打包"
echo " 版本: $VERSION"
echo " 项目根目录: $ROOT"
echo "========================================"

# ---- Step 1: 编译 C++ 服务端 ----
echo ""
echo "[1/4] 编译 FlychessServer (Release)..."
cd "$BUILD_DIR"
cmake --build . --config Release
echo "[1/4] ✓ 编译完成"

# ---- Step 2: 复制服务端到打包目录 ----
echo ""
echo "[2/4] 复制服务端到 electron-client/server-bin/..."
rm -rf "$SERVER_BIN_DIR"
mkdir -p "$SERVER_BIN_DIR/config"

# 查找 exe（Release 可能在子目录）
EXE_PATH="$BUILD_DIR/server/FlychessServer.exe"
if [ ! -f "$EXE_PATH" ]; then
    EXE_PATH=$(find "$BUILD_DIR/server" -name "FlychessServer.exe" | head -1)
fi

if [ ! -f "$EXE_PATH" ]; then
    echo "[错误] 找不到 FlychessServer.exe"
    exit 1
fi

cp "$EXE_PATH" "$SERVER_BIN_DIR/"
if [ -d "$BUILD_DIR/server/config" ]; then
    cp -r "$BUILD_DIR/server/config/"* "$SERVER_BIN_DIR/config/"
elif [ -d "$ROOT/config" ]; then
    cp -r "$ROOT/config/"* "$SERVER_BIN_DIR/config/"
fi

echo "[2/4] ✓ 服务端已复制"
echo "  exe:    $(ls "$SERVER_BIN_DIR"/*.exe)"
echo "  config: $(ls "$SERVER_BIN_DIR/config/")"

# ---- Step 3: 更新版本号 ----
echo ""
echo "[3/4] 更新版本号..."
cd "$ELECTRON_DIR"
# 用临时脚本更新 package.json 中的 version
node -e "
const p = require('./package.json');
p.version = '$VERSION';
require('fs').writeFileSync('package.json', JSON.stringify(p, null, 2) + '\n');
"
echo "[3/4] ✓ 版本号: $VERSION"

# ---- Step 4: 打包 Electron ----
echo ""
echo "[4/4] 打包 Electron 客户端..."
cd "$ELECTRON_DIR"

# 检查依赖
if [ ! -d "node_modules" ]; then
    echo "  安装 npm 依赖..."
    npm install
fi

npm run build
echo "[4/4] ✓ 打包完成"

# ---- 输出结果 ----
echo ""
echo "========================================"
echo " Release 打包完成!"
echo " 版本: $VERSION"
echo " 输出: $ELECTRON_DIR/dist/"
echo "========================================"
ls -lh "$ELECTRON_DIR/dist/"*.exe 2>/dev/null || ls -lh "$ELECTRON_DIR/dist/"
