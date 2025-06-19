document.addEventListener('DOMContentLoaded', () => {
  const startBtn = document.getElementById('start-btn');
  const gameContainer = document.getElementById('game-container');
  const playerSelect = document.getElementById('player-count-select');
  const chessNumSelect = document.getElementById('chess-count-select');

  startBtn.addEventListener('click', () => {
    startBtn.style.display = 'none';
    playerSelect.style.display = 'block';
  });

  let playerCount = 0;

  playerSelect.addEventListener('click',(event)=>{
    const target = event.target;
    if (target.tagName === 'BUTTON' && target.dataset.player) {
      playerCount = parseInt(target.dataset.player);

      playerSelect.style.display = 'none';
      chessNumSelect.style.display = 'block';
    }
  });

  chessNumSelect.addEventListener('click',(event)=>{
    const target = event.target;
    if (target.tagName === 'BUTTON' && target.dataset.chess) {
      const chess_per_player = parseInt(target.dataset.chess);

      chessNumSelect.style.display = 'none';
      gameContainer.style.display = 'block';

      // 初始化游戏，传入玩家人数
      initGame(playerCount, chess_per_player);
    }
  });

});

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

const ChessColor = {
  0: "gray",
  1: "#ff4d4d",   // 比地图红更鲜明
  2: "#4d4dff",   // 更深的蓝色
  3: "#33cc33",   // 更亮的绿色
  4: "#cccc00"    // 更浓的黄色
};

function initGame(playerCount, chess_per_player) {
  FlyChessModule().then(Module => {
    // console.log(`初始化游戏，玩家人数: ${playerCount}`);
    Module._GameInit(playerCount, chess_per_player);

    drawMap(Module); // 初始绘制地图

    gameProcess(Module, playerCount, chess_per_player); // 启动游戏逻辑
  });
}

 function gameProcess(Module, playerCount, chess_per_player) {
  let actual_playerCount = Module._GetPlayerCount();
  let actual_chess_per_player = Module._GetChessPieceCount();
  // console.log(`实际玩家人数: ${actual_playerCount}, 每人棋子数: ${actual_chess_per_player}`);

  // 检查实际玩家人数和棋子数是否与预期一致
  if (actual_playerCount !== playerCount || actual_chess_per_player !== chess_per_player) {
    console.error("实际玩家人数或棋子数与预期不一致，请检查初始化参数。");
    return;
  }
  console.log(`游戏正式开始,玩家人数: ${playerCount},每人棋子数: ${chess_per_player}`);

  // 绘制棋子
  drawChessPieces(Module, playerCount, chess_per_player);

  // 掷骰子按钮绑定
  document.getElementById('roll-btn').onclick = () => {
    showRandomNumber();
  };

  function showRandomNumber() {
    const randomNumber = Module._rollDice();
    console.log(`掷骰子结果: ${randomNumber}`);
    document.getElementById('random-number').textContent = randomNumber;
  }
}

function drawChessPieces(Module, player_count, chess_per_player) {
  // 刷新方式待定
  drawMap(Module); // 重新绘制地图

  const canvas = document.getElementById('flychess-map');
  const ctx = canvas.getContext('2d');

  // 填满预留的圆
  const radius = canvas.width / 51; // 半径为宽高的四分之一

  for (let i = 0; i < player_count; i++) {
    for (let j = 0; j < chess_per_player; j++) {
      // 获取棋子位置
      const ptr = Module._DrawChessPiece(i, j);
      if (ptr != 0) {
        const HEAP32 = Module.HEAP32;
        const base = ptr >> 2; // 转换为 32-bit 索引, 因为 HEAP32 是按 4 字节对齐的，所以除以 4
  
        const type       = HEAP32[base + 0]; // GridType
        const position_x = HEAP32[base + 1];
        const position_y = HEAP32[base + 2];
        const id         = HEAP32[base + 3];
        const width      = HEAP32[base + 4];
        const height     = HEAP32[base + 5];
        const color      = HEAP32[base + 6];
        
        const fillColor = ChessColor[color + 1] || "gray";

        // 计算棋子中心位置
        let center_x = position_x + width / 2;
        let center_y = position_y + height / 2;
        
        if (type == 0 || type == 2){
          ctx.beginPath();
          ctx.arc(center_x, center_y, radius, 0, Math.PI * 2);
          ctx.fillStyle = fillColor; // 圆的填充颜色
          ctx.fill();
          ctx.stroke();
        }
        else if (type == 3) {
            // 按照方向修正中心点位置
            if (height == 0) {
              center_y -= width / 1.5;
            } else if (height == 1) {
              center_x += width / 1.5;
            } else if (height == 2) {
              center_y += width / 1.5;
            } else if (height == 3) {
              center_x -= width / 1.5;
            }

            ctx.beginPath();
            ctx.arc(center_x, center_y, radius, 0, Math.PI * 2);
            ctx.fillStyle = fillColor; // 圆的填充颜色
            ctx.fill();
            ctx.stroke();
        }
        else {
          let x1 = position_x, y1 = position_y;
          let x2, y2, x3, y3;
          
          if (height == 0) {
            x2 = position_x + width;
            y2 = position_y;
            x3 = position_x;
            y3 = position_y + width;
          }
          else if (height == 1) {
            x2 = position_x - width;
            y2 = position_y;
            x3 = position_x;
            y3 = position_y + width;
          }
          else if (height == 2) {
            x2 = position_x;
            y2 = position_y - width;
            x3 = position_x - width;
            y3 = position_y;
          }
          else if (height == 3) {
            x2 = position_x;
            y2 = position_y - width;
            x3 = position_x + width;
            y3 = position_y;
          }
          
          // 计算三角形中心点（圆心）
          let cx = (x1 + x2 + x3) / 3;
          let cy = (y1 + y2 + y3) / 3;
          
          // 画圆
          ctx.beginPath();
          ctx.arc(cx, cy, radius, 0, Math.PI * 2);
          ctx.fillStyle = fillColor; // 圆的填充颜色
          ctx.fill();
          ctx.stroke();
        }
        
      }
    }
  }
  
}

// 画地图函数
function drawMap(Module) {
  console.log(`绘制地图...`);
  const canvas = document.getElementById('flychess-map');
  const ctx = canvas.getContext('2d');

  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.strokeStyle = '#555';

  const grid_num = Module._GetGridCount();
  const radius = canvas.width / 51; // 半径为宽高的四分之一
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

      // console.log(`格子信息: 类型=${GridType[type]}, 坐标=(${position_x},${position_y}), ID=${id}, 宽=${width}, 高=${height}, 颜色=${colorName}`);
      
      if(type == 0 || type == 2){
        // 绘制格子
        ctx.fillStyle = fillColor;
        ctx.fillRect(position_x, position_y, width, height);
        ctx.strokeRect(position_x, position_y, width, height);

        // 绘制空心圆
        let center_x = position_x + width / 2;
        let center_y = position_y + height / 2;
        ctx.beginPath();
        ctx.arc(center_x, center_y, radius, 0, Math.PI * 2);
        ctx.fillStyle = "white"; // 圆的填充颜色
        ctx.fill();
        ctx.stroke();
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

        let center_x = position_x;
        let center_y = position_y;

        // 按照方向修正中心点位置
        if (height == 0) {
          center_y -= width / 1.5;
        } else if (height == 1) {
          center_x += width / 1.5;
        } else if (height == 2) {
          center_y += width / 1.5;
        } else if (height == 3) {
          center_x -= width / 1.5;
        }

        ctx.beginPath();
        ctx.arc(center_x, center_y, radius, 0, Math.PI * 2);
        ctx.fillStyle = "white";
        ctx.fill();
        ctx.stroke();
      }
      else{
        ctx.beginPath();
        ctx.moveTo(position_x, position_y); // 顶点
        
        let x1 = position_x, y1 = position_y;
        let x2, y2, x3, y3;
        
        if (height == 0) {
          x2 = position_x + width;
          y2 = position_y;
          x3 = position_x;
          y3 = position_y + width;
        }
        else if (height == 1) {
          x2 = position_x - width;
          y2 = position_y;
          x3 = position_x;
          y3 = position_y + width;
        }
        else if (height == 2) {
          x2 = position_x;
          y2 = position_y - width;
          x3 = position_x - width;
          y3 = position_y;
        }
        else if (height == 3) {
          x2 = position_x;
          y2 = position_y - width;
          x3 = position_x + width;
          y3 = position_y;
        }
        
        ctx.lineTo(x2, y2);
        ctx.lineTo(x3, y3);
        ctx.closePath();
        
        // 画三角形
        ctx.fillStyle = fillColor;
        ctx.fill();
        ctx.stroke();
        
        // 计算三角形中心点（圆心）
        let cx = (x1 + x2 + x3) / 3;
        let cy = (y1 + y2 + y3) / 3;
        
        // 画圆
        ctx.beginPath();
        ctx.arc(cx, cy, radius, 0, Math.PI * 2);
        ctx.fillStyle = "white";
        ctx.fill();
        ctx.stroke();
      }
    }
  }
}
