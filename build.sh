set -e  # 脚本遇错自动退出，避免遗漏错误

./server_build.sh

./wasm_build.sh

echo "✅ 编译全部完成"
