set -e  # 脚本遇错自动退出，避免遗漏错误

cd flychess_game || { echo "❌ 进入 flychess_game 目录失败"; exit 1; }

# 使用emcmake配置build目录（可以加上 -DCMAKE_BUILD_TYPE=Release 做release构建）
emcmake cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 编译
emmake make -C build

# 复制 wasm 和 js 文件到 frontend 目录
cp build/game.* ../frontend/

echo "✅ 编译完成并复制到 frontend/ 目录"
