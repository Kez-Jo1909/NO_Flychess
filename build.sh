cd flychess_game || exit 1

# 使用emcmake创建build目录
emcmake cmake -S . -B build

# 编译，判断是否成功
if emmake make -C build; then
  # 编译成功才复制文件
  cp build/game.* ../frontend/
  echo "✅ 编译完成并复制到 frontend/ 目录"
else
  echo "❌ 编译失败，未复制文件"
  exit 1
fi
