// ============================================================
// 骰子显示
// ============================================================
const DiceUI = {
  display: null,
  rollBtn: null,
  value: 0,

  init(displayId, btnId) {
    this.display = document.getElementById(displayId);
    this.rollBtn = document.getElementById(btnId);
  },

  setValue(v) {
    this.value = v;
    if (this.display) {
      this.display.textContent = v > 0 ? v : '?';
    }
  },

  enableRoll(callback) {
    if (this.rollBtn) {
      this.rollBtn.disabled = false;
      this.rollBtn.textContent = '掷骰子';
      this.rollBtn.onclick = callback;
    }
  },

  disableRoll(text) {
    if (this.rollBtn) {
      this.rollBtn.disabled = true;
      this.rollBtn.textContent = text || '等待中...';
    }
  }
};
