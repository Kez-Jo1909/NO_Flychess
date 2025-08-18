set -e  # 脚本遇错自动退出，避免遗漏错误

mkdir -p build

cd build

cmake ..

make -j16

cd ..

# ./wasm_build.sh

echo "✅ 编译全部完成"
