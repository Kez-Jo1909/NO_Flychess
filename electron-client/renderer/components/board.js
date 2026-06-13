// ============================================================
// Canvas 棋盘渲染
// 地图数据从 config/game_map.json 加载，棋子位置从服务端推送
// ============================================================
const Board = {
  canvas: null,
  ctx: null,
  grids: [],          // 地图格子数据
  pieces: [],         // 棋子数据 [{color,id,position,player_id}]
  selectedPiece: null,// {player_id, piece_id}
  onPieceClick: null, // 回调
  cellSize: 40,       // 每个格子的基础尺寸

  colors: {
    0: '#cccccc', 1: '#ff4d4d', 2: '#4d4dff',
    3: '#33cc33', 4: '#cccc00', '-1': '#e0e0e0'
  },

  pieceColors: {
    0: '#888888', 1: '#ff1a1a', 2: '#1a1aff',
    3: '#1ab21a', 4: '#b3b300'
  },

  async init(canvasId) {
    this.canvas = document.getElementById(canvasId);
    this.ctx = this.canvas.getContext('2d');

    // 通过 IPC 加载地图数据（绕过 file:// CORS 限制）
    try {
      const result = await window.electronAPI.loadConfig('game_map.json');
      if (result.success) {
        this.grids = result.data.grids || [];
        console.log('[Board] 加载地图:', this.grids.length, '个格子');
      } else {
        console.error('[Board] 地图加载失败:', result.error);
      }
    } catch (e) {
      console.error('[Board] 地图加载异常:', e);
    }

    this.draw();

    // 点击事件
    this.canvas.addEventListener('click', (e) => {
      const rect = this.canvas.getBoundingClientRect();
      const mx = e.clientX - rect.left;
      const my = e.clientY - rect.top;
      this._hitTest(mx, my);
    });
  },

  // 设置棋子数据（服务端推送）
  setPieces(pieceList) {
    this.pieces = pieceList;  // [{id, color, position, player_id}]
    this.draw();
  },

  // 画整个棋盘
  draw() {
    const ctx = this.ctx;
    const w = this.canvas.width;
    const h = this.canvas.height;
    ctx.clearRect(0, 0, w, h);
    ctx.fillStyle = '#f5f0e8';
    ctx.fillRect(0, 0, w, h);

    // 画格子
    for (const g of this.grids) {
      if (g.id === -2) continue;  // 未使用
      this._drawGrid(g);
    }

    // 画棋子
    for (const p of this.pieces) {
      if (p.position === -1 || p.position === -2) continue;  // 在家或已完成
      this._drawPiece(p);
    }
  },

  _drawGrid(g) {
    const ctx = this.ctx;
    const x = g.position_x, y = g.position_y;
    const w = g.width, h = g.height || g.width;
    const fillColor = this.colors[g.color] || '#ccc';

    ctx.strokeStyle = '#444';
    ctx.lineWidth = 1;

    if (g.type === 0 || g.type === 2) {
      // 矩形
      ctx.fillStyle = fillColor;
      ctx.fillRect(x, y, w, h);
      ctx.strokeRect(x, y, w, h);
      // 圆心
      const cx = x + w/2, cy = y + h/2;
      const r = w * 0.32;
      ctx.beginPath();
      ctx.arc(cx, cy, r, 0, Math.PI * 2);
      ctx.fillStyle = '#fff';
      ctx.fill();
      ctx.stroke();
    } else if (g.type === 3) {
      // 中心三角
      this._drawTriangle(ctx, x, y, w, g.height, fillColor);
      const [cx, cy] = this._triangleCenter(x, y, w, g.height);
      const r = w * 0.28;
      ctx.beginPath();
      ctx.arc(cx, cy, r, 0, Math.PI * 2);
      ctx.fillStyle = '#fff';
      ctx.fill();
      ctx.stroke();
    } else {
      // 角落三角
      this._drawCornerTriangle(ctx, x, y, w, g.height, fillColor);
    }
  },

  _drawTriangle(ctx, x, y, w, dir, color) {
    ctx.beginPath();
    ctx.moveTo(x, y);
    if (dir === 0) { ctx.lineTo(x - w, y - w); ctx.lineTo(x + w, y - w); }
    else if (dir === 1) { ctx.lineTo(x + w, y - w); ctx.lineTo(x + w, y + w); }
    else if (dir === 2) { ctx.lineTo(x - w, y + w); ctx.lineTo(x + w, y + w); }
    else { ctx.lineTo(x - w, y + w); ctx.lineTo(x - w, y - w); }
    ctx.closePath();
    ctx.fillStyle = color;
    ctx.fill();
    ctx.stroke();
  },

  _drawCornerTriangle(ctx, x, y, w, dir, color) {
    let x2, y2, x3, y3;
    if (dir === 0) { x2 = x + w; y2 = y; x3 = x; y3 = y + w; }
    else if (dir === 1) { x2 = x - w; y2 = y; x3 = x; y3 = y + w; }
    else if (dir === 2) { x2 = x; y2 = y - w; x3 = x - w; y3 = y; }
    else { x2 = x; y2 = y - w; x3 = x + w; y3 = y; }

    ctx.beginPath();
    ctx.moveTo(x, y);
    ctx.lineTo(x2, y2);
    ctx.lineTo(x3, y3);
    ctx.closePath();
    ctx.fillStyle = color;
    ctx.fill();
    ctx.stroke();

    // 圆心
    const cx = (x + x2 + x3) / 3;
    const cy = (y + y2 + y3) / 3;
    const r = w * 0.28;
    ctx.beginPath();
    ctx.arc(cx, cy, r, 0, Math.PI * 2);
    ctx.fillStyle = '#fff';
    ctx.fill();
    ctx.stroke();
  },

  _triangleCenter(x, y, w, dir) {
    if (dir === 0) return [x, y - w / 1.5];
    if (dir === 1) return [x + w / 1.5, y];
    if (dir === 2) return [x, y + w / 1.5];
    return [x - w / 1.5, y];
  },

  // 获取格子的圆心像素坐标
  _getGridCenter(g) {
    const x = g.position_x, y = g.position_y;
    const w = g.width, h = g.height || g.width;
    if (g.type === 0 || g.type === 2) return [x + w/2, y + h/2];
    if (g.type === 3) return this._triangleCenter(x, y, w, h);
    // 角落三角
    let x2, y2, x3, y3;
    if (h === 0) { x2 = x + w; y2 = y; x3 = x; y3 = y + w; }
    else if (h === 1) { x2 = x - w; y2 = y; x3 = x; y3 = y + w; }
    else if (h === 2) { x2 = x; y2 = y - w; x3 = x - w; y3 = y; }
    else { x2 = x; y2 = y - w; x3 = x + w; y3 = y; }
    return [(x + x2 + x3) / 3, (y + y2 + y3) / 3];
  },

  _drawPiece(p) {
    // 通过 position + color 找到对应格子的圆心
    // position 是逻辑位置 ID，color 是棋子所有者颜色
    const grid = this.grids.find(g => g.id === p.position && g.color === p.color);
    if (!grid && p.position >= 0) {
      // 可能在共享路径上（position 1-52），找匹配 position 的普通格子
      const shared = this.grids.find(g => g.id === p.position && (g.type === 0));
      if (shared) {
        this._drawPieceAt(shared, p);
      }
      return;
    }
    if (grid) {
      this._drawPieceAt(grid, p);
    }
  },

  _drawPieceAt(grid, p) {
    const [cx, cy] = this._getGridCenter(grid);
    const r = (grid.width || this.cellSize) * 0.28;
    const color = this.pieceColors[p.color] || '#888';

    const ctx = this.ctx;
    ctx.beginPath();
    ctx.arc(cx, cy, r, 0, Math.PI * 2);
    ctx.fillStyle = color;
    ctx.fill();
    ctx.strokeStyle = '#333';
    ctx.lineWidth = 2;
    ctx.stroke();

    // 选中高亮
    if (this.selectedPiece &&
        this.selectedPiece.player_id === p.player_id &&
        this.selectedPiece.piece_id === p.id) {
      ctx.beginPath();
      ctx.arc(cx, cy, r + 3, 0, Math.PI * 2);
      ctx.strokeStyle = '#e94560';
      ctx.lineWidth = 3;
      ctx.stroke();
    }
  },

  _hitTest(mx, my) {
    if (!this.onPieceClick) return;

    for (const p of this.pieces) {
      if (p.position < 0) continue;
      const grid = this._findGridForPiece(p);
      if (!grid) continue;

      const [cx, cy] = this._getGridCenter(grid);
      const r = (grid.width || this.cellSize) * 0.32;
      const dx = mx - cx, dy = my - cy;
      if (dx * dx + dy * dy <= r * r) {
        this.selectedPiece = { player_id: p.player_id, piece_id: p.id };
        this.draw();
        this.onPieceClick(p.player_id, p.id);
        return;
      }
    }
    this.selectedPiece = null;
    this.draw();
  },

  _findGridForPiece(p) {
    // 先精确匹配
    let g = this.grids.find(g => g.id === p.position && g.color === p.color);
    if (g) return g;
    // 共享路径
    g = this.grids.find(g => g.id === p.position && g.type === 0);
    return g || null;
  }
};
