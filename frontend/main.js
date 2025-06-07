FlyChessModule().then(Module => {
  const canvas = document.getElementById('flychess-map');
  const ctx = canvas.getContext('2d');

  const size = 40; // 格子大小
  const rows = 15;
  const cols = 15;

  // 画地图函数
  function drawMap() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.strokeStyle = '#555';
    for (let r = 0; r < rows; r++) {
      for (let c = 0; c < cols; c++) {
        ctx.strokeRect(c * size, r * size, size, size);
      }
    }
    ctx.fillStyle = 'yellow';
    ctx.fillRect(0, 0, size, size);
    ctx.fillStyle = 'red';
    ctx.fillRect((cols - 1) * size, (rows - 1) * size, size, size);
  }

  // 显示随机数
  function showRandomNumber() {
    // 调用 wasm 的 GetRandom 函数（函数名带下划线）
    const random = Module._GetRandom();
    // 显示到页面 p 标签
    document.getElementById('random-number').innerText = `掷骰子点数：${random}`;

    // 也可以画到 Canvas 上
    ctx.font = '24px sans-serif';
    ctx.fillStyle = 'black';
    ctx.fillText(`点数: ${random}`, 10, canvas.height - 20);
  }

  // 初始化页面绘制
  drawMap();

  // 绑定按钮点击事件
  document.getElementById('roll-btn').onclick = () => {
    showRandomNumber();
  };
});
