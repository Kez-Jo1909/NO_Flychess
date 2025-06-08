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

    // 画框
    for (let c = 0; c < cols; c++) {
      ctx.strokeRect(c * size, 0 * size, size, size);
      ctx.strokeRect(c * size, (rows - 1) * size, size, size);
    }
    for (let r = 1; r < rows - 1; r++) {
      ctx.strokeRect(0 * size, r * size, size, size);
      ctx.strokeRect((cols - 1) * size, r * size, size, size);
    }

    // 以下角点
    ctx.fillStyle = 'red';
    ctx.fillRect(0, 0, size, size);
    ctx.fillStyle = 'blue';
    ctx.fillRect(0, (rows - 1) * size, size, size);
    ctx.fillStyle = 'green';
    ctx.fillRect((cols - 1) * size, (rows - 1) * size, size, size);
    ctx.fillStyle = 'yellow';
    ctx.fillRect((cols - 1) * size, 0, size, size);
  }

  // 显示随机数
  function showRandomNumber() {
    const random = Module._GetRandom();
    document.getElementById('random-number').innerText = `掷骰子点数：${random}`;
  
    // 只清除原来文字所在区域
    const x = 0;
    const y = 0; // 上移一点，确保文字完全清除
    const w = 600;
    const h = 600;
    ctx.clearRect(x, y, w, h);
    
    // 重新绘制地图
    drawMap();

    // 重新绘制文字
    ctx.font = '24px sans-serif';
    ctx.fillStyle = 'black';
    ctx.fillText(`点数: ${random}`, 220, 300);

    Module._GameProcessLink();
  }
  

  // 初始化页面绘制
  drawMap();

  // 绑定按钮点击事件
  document.getElementById('roll-btn').onclick = () => {
    showRandomNumber();
  };
});
