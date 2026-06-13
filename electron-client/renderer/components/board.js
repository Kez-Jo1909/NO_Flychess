// ============================================================
// Canvas 棋盘渲染 — 完全复刻 C++ Map::searchGridInfo 逻辑
// ============================================================
const Board = {
  canvas: null, ctx: null, grids: [], pieces: [],
  selectedPiece: null, onPieceClick: null,
  boardSize: 0, gridSize: 0, radius: 0, offsetX: 0, offsetY: 0,

  _colorRGB(c) {
    switch (c) {
      case 0: return [255, 0, 0];
      case 1: return [0, 0, 255];
      case 2: return [0, 255, 0];
      case 3: return [255, 255, 0];
      default: return [204, 204, 0];
    }
  },

  _rgbStr(c) {
    const [r, g, b] = this._colorRGB(c);
    return `rgb(${r},${g},${b})`;
  },

  _getGridCenter(px, py, width, height, type) {
    let cx = px, cy = py;
    if (type === 0 || type === 2) { cx += width / 2; cy += height / 2; }
    else if (type === 3) {
      if (height === 0) cy -= width / 1.5;
      else if (height === 1) cx += width / 1.5;
      else if (height === 2) cy += width / 1.5;
      else cx -= width / 1.5;
    } else {
      if (height === 0) { cx += width / 3.0; cy += width / 3.0; }
      else if (height === 1) { cx -= width / 3.0; cy += width / 3.0; }
      else if (height === 2) { cx -= width / 3.0; cy -= width / 3.0; }
      else { cx += width / 3.0; cy -= width / 3.0; }
    }
    return [Math.round(cx), Math.round(cy)];
  },

  // === 完全复刻 C++ Map::searchGridInfo(position, color, chess_id) ===
  _searchGridInfo(position, color, pieceId) {
    if (position === -1) {
      // HOME: index = color + chess_id * 4
      return this.grids[color + pieceId * 4] || null;
    }
    if (position === -2) {
      return this.grids[96] || null;
    }
    if (position === 58) {
      // GOAL: index = color + 88
      return this.grids[color + 88] || null;
    }
    if (position === 0) {
      // START: index = 92 + color
      return this.grids[92 + color] || null;
    }
    if (position >= 1 && position <= 52) {
      // NORMAL/TURN/BRIDGE: 遍历 indices 16~87（跳过 53~57 pre-goal），匹配 grid.id
      for (let i = 16; i <= 87; i++) {
        if (i > 52 && i < 58) continue;
        const g = this.grids[i];
        if (g && g.id === position) return g;
      }
      console.warn('[Board] NORMAL MISS: pos=', position);
      return null;
    }
    if (position > 52 && position < 58) {
      // PRE-GOAL: index = 52 + color + (position - 53) * 4
      return this.grids[52 + color + (position - 53) * 4] || null;
    }
    console.warn('[Board] searchGridInfo MISS:', position, color);
    return null;
  },

  // START 格子颜色用 height 映射（JSON 里 color=-1）
  _gridDisplayColor(g) {
    if (g.id === 0 && g.type === 1) return this._colorRGB(g.height);
    return this._colorRGB(g.color);
  },

  async init(canvasId) {
    this.canvas = document.getElementById(canvasId);
    this.ctx = this.canvas.getContext('2d');
    const result = await window.electronAPI.loadConfig('game_map.json');
    if (result.success) this.grids = result.data.grids || [];
    this.draw();
    this.canvas.addEventListener('click', (e) => {
      const rect = this.canvas.getBoundingClientRect();
      this._hitTest(e.clientX - rect.left, e.clientY - rect.top);
    });
  },

  setPieces(pieceList) { this.pieces = pieceList; this.draw(); },

  draw() {
    const ctx = this.ctx;
    if (!ctx) return;
    const cw = this.canvas.width, ch = this.canvas.height;

    this.boardSize = Math.floor(Math.min(cw, ch) / 34) * 34;
    this.offsetX = Math.floor((cw - this.boardSize) / 2);
    this.offsetY = Math.floor((ch - this.boardSize) / 2);
    this.gridSize = this.boardSize / 17;
    this.radius = this.boardSize / 51;

    ctx.clearRect(0, 0, cw, ch);
    ctx.fillStyle = '#fff'; ctx.fillRect(0, 0, cw, ch);
    ctx.fillRect(this.offsetX, this.offsetY, this.boardSize, this.boardSize);
    ctx.strokeStyle = '#000'; ctx.lineWidth = 2;
    ctx.strokeRect(this.offsetX, this.offsetY, this.boardSize, this.boardSize);

    for (const g of this.grids) {
      if (g.id === -2) continue;
      const type = g.type;
      let px = Math.round(g.position_x / 40 * this.gridSize) + this.offsetX;
      let py = Math.round(g.position_y / 40 * this.gridSize) + this.offsetY;
      const dispRgb = this._gridDisplayColor(g);
      const fill = `rgb(${dispRgb[0]},${dispRgb[1]},${dispRgb[2]})`;
      ctx.strokeStyle = '#000'; ctx.lineWidth = 1;

      if (type === 0 || type === 2) {
        const w = Math.round(g.width / 40 * this.gridSize);
        const h = Math.round(g.height / 40 * this.gridSize);
        ctx.fillStyle = fill; ctx.fillRect(px, py, w, h); ctx.strokeRect(px, py, w, h);
        const [cx, cy] = this._getGridCenter(px, py, w, h, type);
        ctx.beginPath(); ctx.arc(cx, cy, this.radius, 0, Math.PI*2);
        ctx.fillStyle = '#fff'; ctx.fill(); ctx.stroke();
      } else if (type === 3) {
        px = this.offsetX + this.boardSize / 2; py = this.offsetY + this.boardSize / 2;
        const w = g.width / 40.0 * this.gridSize;
        this._drawCenterTriangle(ctx, px, py, w, g.height, fill);
        const [cx, cy] = this._getGridCenter(px, py, w, g.height, type);
        ctx.beginPath(); ctx.arc(cx, cy, this.radius, 0, Math.PI*2);
        ctx.fillStyle = '#fff'; ctx.fill(); ctx.stroke();
      } else {
        const w = g.width / 40.0 * this.gridSize;
        this._drawCornerTriangle(ctx, px, py, w, g.height, fill);
        const [cx, cy] = this._getGridCenter(px, py, w, g.height, type);
        ctx.beginPath(); ctx.arc(cx, cy, this.radius, 0, Math.PI*2);
        ctx.fillStyle = '#fff'; ctx.fill(); ctx.stroke();
      }
    }

    for (const p of this.pieces) {
      const grid = this._searchGridInfo(p.position, p.color, p.id);
      if (!grid || grid.id === -2) continue;

      let px = Math.round(grid.position_x / 40 * this.gridSize) + this.offsetX;
      let py = Math.round(grid.position_y / 40 * this.gridSize) + this.offsetY;
      let width, type = grid.type;

      if (type === 3) {
        px = this.offsetX + this.boardSize / 2; py = this.offsetY + this.boardSize / 2;
        width = grid.width / 40.0 * this.gridSize;
      } else if (type === 0 || type === 2) {
        width = Math.round(grid.width / 40 * this.gridSize);
      } else {
        width = grid.width / 40.0 * this.gridSize;
      }

      const [cx, cy] = this._getGridCenter(px, py, width,
        (type === 0 || type === 2) ? Math.round(grid.height / 40 * this.gridSize) : grid.height, type);
      const prgb = this._colorRGB(p.color);
      ctx.beginPath(); ctx.arc(cx, cy, this.radius, 0, Math.PI*2);
      ctx.fillStyle = `rgb(${prgb[0]},${prgb[1]},${prgb[2]})`; ctx.fill();
      ctx.strokeStyle = '#000'; ctx.lineWidth = 1; ctx.stroke();

      if (this.selectedPiece && this.selectedPiece.player_id === p.player_id && this.selectedPiece.piece_id === p.id) {
        ctx.beginPath(); ctx.arc(cx, cy, this.radius + 3, 0, Math.PI*2);
        ctx.strokeStyle = '#e94560'; ctx.lineWidth = 3; ctx.stroke();
      }
    }
  },

  _drawCenterTriangle(ctx, x, y, w, dir, color) {
    ctx.beginPath(); ctx.moveTo(x, y);
    if (dir === 0) { ctx.lineTo(x - w, y - w); ctx.lineTo(x + w, y - w); }
    else if (dir === 1) { ctx.lineTo(x + w, y - w); ctx.lineTo(x + w, y + w); }
    else if (dir === 2) { ctx.lineTo(x - w, y + w); ctx.lineTo(x + w, y + w); }
    else { ctx.lineTo(x - w, y + w); ctx.lineTo(x - w, y - w); }
    ctx.closePath(); ctx.fillStyle = color; ctx.fill(); ctx.stroke();
  },

  _drawCornerTriangle(ctx, x, y, w, dir, color) {
    ctx.beginPath(); ctx.moveTo(x, y);
    if (dir === 0) { ctx.lineTo(x + w, y); ctx.lineTo(x, y + w); }
    else if (dir === 1) { ctx.lineTo(x - w, y); ctx.lineTo(x, y + w); }
    else if (dir === 2) { ctx.lineTo(x, y - w); ctx.lineTo(x - w, y); }
    else { ctx.lineTo(x, y - w); ctx.lineTo(x + w, y); }
    ctx.closePath(); ctx.fillStyle = color; ctx.fill(); ctx.stroke();
  },

  _hitTest(mx, my) {
    if (!this.onPieceClick) return;
    for (const p of this.pieces) {
      const grid = this._searchGridInfo(p.position, p.color, p.id);
      if (!grid || grid.id === -2) continue;
      let px = Math.round(grid.position_x / 40 * this.gridSize) + this.offsetX;
      let py = Math.round(grid.position_y / 40 * this.gridSize) + this.offsetY;
      let width, type = grid.type;
      if (type === 3) { px = this.offsetX + this.boardSize / 2; py = this.offsetY + this.boardSize / 2; width = grid.width / 40.0 * this.gridSize; }
      else if (type === 0 || type === 2) { width = Math.round(grid.width / 40 * this.gridSize); }
      else { width = grid.width / 40.0 * this.gridSize; }
      const [cx, cy] = this._getGridCenter(px, py, width,
        (type === 0 || type === 2) ? Math.round(grid.height / 40 * this.gridSize) : grid.height, type);
      if ((mx-cx)**2 + (my-cy)**2 <= this.radius**2) {
        this.selectedPiece = { player_id: p.player_id, piece_id: p.id };
        this.draw(); this.onPieceClick(p.player_id, p.id); return;
      }
    }
    this.selectedPiece = null; this.draw();
  }
};
