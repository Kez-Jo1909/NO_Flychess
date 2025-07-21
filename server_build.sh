set -e  # 脚本遇错自动退出，避免遗漏错误

cd flychess_game || { echo "❌ 进入 flychess_game 目录失败"; exit 1; }

mkdir -p build
cd build

cmake ..

make -j16

echo "✅ server所需库编译完成"
