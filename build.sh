cd flychess_game
#使用emcmake创建目录
emcmake cmake -S . -B build
#编译
emmake make -C build
#复制编译好的文件(看起来有点屎)
cp build/game.* ../frontend/
echo "✅ 编译完成并复制到 frontend/ 目录"
