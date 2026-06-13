// ============================================================
// 卡牌 UI — 手牌显示
// ============================================================
const CardUI = {
  cards: [],        // 持有的卡牌 [{id, name, description, image_path}]
  selectedIdx: -1,  // 当前选中的卡牌索引
  container: null,

  init(containerId) {
    this.container = document.getElementById(containerId);
  },

  // 新增卡牌（服务端推送 get_card）
  addCard(cardInfo) {
    this.cards.push(cardInfo);
    this.render();
  },

  // 移除已使用的卡牌
  removeCard(index) {
    if (index >= 0 && index < this.cards.length) {
      this.cards.splice(index, 1);
      this.selectedIdx = -1;
      this.render();
    }
  },

  // 获取当前选中的卡牌
  getSelected() {
    if (this.selectedIdx >= 0 && this.selectedIdx < this.cards.length) {
      return this.cards[this.selectedIdx];
    }
    return null;
  },

  clear() {
    this.cards = [];
    this.selectedIdx = -1;
    this.render();
  },

  render() {
    if (!this.container) return;
    this.container.innerHTML = '';

    if (this.cards.length === 0) {
      this.container.innerHTML = '<p class="sidebar-hint">暂无卡牌</p>';
      return;
    }

    this.cards.forEach((card, idx) => {
      const el = document.createElement('div');
      el.className = 'card-item';
      if (idx === this.selectedIdx) el.classList.add('selected');

      el.innerHTML = `
        <div class="card-img" style="background-image:url('${card.image_path || ''}')"></div>
        <div class="card-name">${card.name}</div>
        <div class="card-desc">${card.description}</div>
      `;

      el.addEventListener('click', () => {
        if (this.selectedIdx === idx) {
          this.selectedIdx = -1;  // 取消选中
        } else {
          this.selectedIdx = idx;
        }
        this.render();
      });

      this.container.appendChild(el);
    });
  }
};
