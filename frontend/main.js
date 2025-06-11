// 1. 启动逻辑：点击“开始游戏”按钮后显示游戏容器，并初始化游戏模块
document.addEventListener('DOMContentLoaded', () => {
  const startBtn = document.getElementById('start-btn');
  const gameContainer = document.getElementById('game-container');

  startBtn.addEventListener('click', () => {
    startBtn.style.display = 'none';
    gameContainer.style.display = 'block';

    // 点击后再初始化游戏
    initGame();
  });
});

// 2. 游戏初始化逻辑（你的 FlyChessModule 和地图绘制逻辑）
function initGame() {
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

    // 掷骰子按钮绑定
    document.getElementById('roll-btn').onclick = () => {
      showRandomNumber();
    };

    drawMap(); // 初始绘制地图

    function showRandomNumber() {
      const randomNumber = Module._rollDice();
      document.getElementById('random-number').textContent = randomNumber;
    }
  });
}
