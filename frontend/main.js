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

    const GridType = {
      0: "NORMAL",
      1: "START",
      2: "HOME",
      3: "GOAL",
      4: "BRIDGE",
      5: "TURN"
    };
    
    const Color = {
      0: { name: "UNDEFINED", draw: "gray" },
      1: { name: "RED",       draw: "red" },
      2: { name: "BLUE",      draw: "#6666ff" },
      3: { name: "GREEN",     draw: "#66ff66" },
      4: { name: "YELLOW",    draw: "yellow" }
    };
    
    

    // 画地图函数
    function drawMap() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      ctx.strokeStyle = '#555';

      const grid_num = Module._GetGridCount();
      for(let i = 0; i < grid_num; i++) {
        // 调用 C++ 获取结构体指针
        const ptr = Module._GetGridInfo(i);

        if(ptr != 0){
          const HEAP32 = Module.HEAP32;
          const base = ptr >> 2; // 转换为 32-bit 索引, 因为 HEAP32 是按 4 字节对齐的，所以除以 4

          const type       = HEAP32[base + 0]; // GridType
          const position_x = HEAP32[base + 1];
          const position_y = HEAP32[base + 2];
          const id         = HEAP32[base + 3];
          const width      = HEAP32[base + 4];
          const height     = HEAP32[base + 5];
          const color      = HEAP32[base + 6];
          
          const colorInfo = Color[color + 1];
          const colorName = colorInfo ? colorInfo.name : "UNDEFINED";
          const fillColor = colorInfo ? colorInfo.draw : "gray";

          console.log(`格子信息: 类型=${GridType[type]}, 坐标=(${position_x},${position_y}), ID=${id}, 宽=${width}, 高=${height}, 颜色=${colorName}`);
          
          if(type == 0 || type == 2){
            // 绘制格子
            ctx.fillStyle = fillColor;
            ctx.fillRect(position_x, position_y, width, height);
            ctx.strokeRect(position_x, position_y, width, height);
          }
          else if(type == 3){
            // 不同类型的三角格子
            ctx.beginPath();
            ctx.moveTo(position_x, position_y);// 顶点
            if(height == 0){
              ctx.lineTo(position_x - width, position_y - width);
              ctx.lineTo(position_x + width, position_y - width);
            }
            else if(height == 1){
              ctx.lineTo(position_x + width, position_y - width);
              ctx.lineTo(position_x + width, position_y + width);
            }
            else if(height == 2){
              ctx.lineTo(position_x - width, position_y + width);
              ctx.lineTo(position_x + width, position_y + width);
            }
            else if(height == 3){
              ctx.lineTo(position_x - width, position_y + width);
              ctx.lineTo(position_x - width, position_y - width);
            }
            ctx.closePath();

            ctx.fillStyle = fillColor;
            ctx.fill();
            ctx.stroke();
          }
          else{
            // 绘制三角形格子
            ctx.beginPath();
            ctx.moveTo(position_x, position_y);// 顶点
            if(height == 0){
              ctx.lineTo(position_x + width, position_y);
              ctx.lineTo(position_x, position_y + width);
            }
            else if(height == 1){
              ctx.lineTo(position_x - width, position_y);
              ctx.lineTo(position_x, position_y + width);
            }
            else if(height == 2){
              ctx.lineTo(position_x, position_y - width);
              ctx.lineTo(position_x - width, position_y);
            }
            else if(height == 3){
              ctx.lineTo(position_x, position_y - width);
              ctx.lineTo(position_x + width, position_y);
            }
            ctx.closePath();

            ctx.fillStyle = fillColor;
            ctx.fill();
            ctx.stroke();
          }
        }
      }
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
