// ============================================================
// 卡牌 UI — 手牌显示 + 弃牌
// ============================================================
const CardUI = {
  cards: [],        // 持有的卡牌 [{id, name, description, image_path, function_time, target_selection}]
  selectedIdx: -1,  // 当前选中的卡牌索引
  container: null,
  maxHandSize: 5,   // 手牌上限
  _pendingUse: null,
  _discardMode: false,
  _discardRequired: false,

  init(containerId) {
    this.container = document.getElementById(containerId);
  },

  // 新增卡牌（服务端推送 get_card）
  // 返回 true 表示成功加入手牌，false 表示手牌已满需要弃牌
  addCard(cardInfo) {
    this.cards.push(cardInfo);
    this.render();
    return true;
  },

  setCards(cards) {
    this.cards = Array.isArray(cards) ? [...cards] : [];
    this.selectedIdx = -1;
    this._pendingUse = null;
    this.render();
  },

  requireDiscard(maxHandSize) {
    this.maxHandSize = maxHandSize || this.maxHandSize;
    this._discardMode = true;
    this._discardRequired = true;
    ChatUI.addSystem('game-chat', `手牌超过 ${this.maxHandSize} 张，请选择一张要弃掉的牌`);
    this.render();
  },

  clearDiscardRequirement() {
    this._discardMode = false;
    this._discardRequired = false;
    this._pendingCard = null;
    this.render();
  },

  // 完成弃牌（保留新卡，丢弃旧卡）
  _finishDiscard(discardIdx) {
    if (!this._discardRequired) return;
    if (discardIdx >= 0 && discardIdx < this.cards.length) {
      const discarded = this.cards[discardIdx];
      WS.send({ type: 'discard_card', card_id: discarded.id });
    }
  },

  // 取消获得新卡（不弃旧卡，放弃新卡）
  _cancelNewCard() {
    if (this._discardRequired) {
      ChatUI.addSystem('game-chat', '当前必须弃掉一张卡牌后才能继续');
      return;
    }
    if (this._pendingCard) {
      ChatUI.addSystem('game-chat', `放弃了新卡牌【${this._pendingCard.name}】`);
    }
    this._discardMode = false;
    this._pendingCard = null;
    this.selectedIdx = -1;
    this.render();
  },

  // 主动弃牌（右键点击卡牌）
  discardCard(index) {
    if (this._discardRequired) {
      this._finishDiscard(index);
      return;
    }
    if (index >= 0 && index < this.cards.length) {
      const card = this.cards[index];
      this.cards.splice(index, 1);
      this.selectedIdx = -1;
      this.render();
      ChatUI.addSystem('game-chat', `弃掉了【${card.name}】（剩余 ${this.cards.length} 张）`);
    }
  },

  // 移除已使用的卡牌
  removeCard(index) {
    console.log('[CardUI] removeCard index=', index, 'cards.length=', this.cards.length,
                'selectedIdx=', this.selectedIdx);
    if (index >= 0 && index < this.cards.length) {
      const removed = this.cards[index];
      this.cards.splice(index, 1);
      this.selectedIdx = -1;
      console.log('[CardUI] 已移除', removed.name, '剩余', this.cards.length, '张');
      this.render();
    } else {
      console.log('[CardUI] removeCard 索引无效! index=', index, 'len=', this.cards.length);
    }
  },

  markPendingUse(index, cardId) {
    if (index < 0 || index >= this.cards.length) return;
    this._pendingUse = { index, card: this.cards[index], cardId };
  },

  commitPendingCard(cardId) {
    if (!this._pendingUse) return;
    if (this._pendingUse.cardId !== cardId) return;
    const pendingIndex = this._pendingUse.index;
    this._pendingUse = null;
    this.removeCard(pendingIndex);
  },

  restorePendingCard(cardId) {
    if (!this._pendingUse) return;
    if (cardId != null && this._pendingUse.cardId !== cardId) return;
    this._pendingUse = null;
    this.render();
  },

  // 获取当前选中的卡牌
  getSelected() {
    if (this.selectedIdx >= 0 && this.selectedIdx < this.cards.length) {
      return this.cards[this.selectedIdx];
    }
    return null;
  },

  // 获取手牌数量
  getHandSize() {
    return this.cards.length;
  },

  getUsableCards(phaseType) {
    return this.cards.filter(card => this._matchesPhase(card.function_time, phaseType));
  },

  getUsableCardCount(phaseType) {
    return this.getUsableCards(phaseType).length;
  },

  clear() {
    this.cards = [];
    this.selectedIdx = -1;
    this._discardMode = false;
    this._discardRequired = false;
    this._pendingCard = null;
    this._pendingUse = null;
    this.render();
  },

  render() {
    if (!this.container) { console.log('[CardUI] render 跳过: container 为空'); return; }
    console.log('[CardUI] render cards.length=', this.cards.length,
                'selectedIdx=', this.selectedIdx,
                'discardMode=', this._discardMode);
    this.container.innerHTML = '';

    if (this.cards.length === 0 && !this._discardMode) {
      this.container.innerHTML = '<p class="sidebar-hint">暂无卡牌</p>';
      return;
    }

    // 手牌数量指示器
    const countInfo = document.createElement('div');
    countInfo.className = 'card-count-info';
    countInfo.textContent = `手牌 ${this.cards.length}/${this.maxHandSize}`;
    if (this.cards.length >= this.maxHandSize) {
      countInfo.classList.add('full');
    }
    this.container.appendChild(countInfo);

    this.cards.forEach((card, idx) => {
      const el = document.createElement('div');
      el.className = 'card-item';
      if (idx === this.selectedIdx) el.classList.add('selected');
      if (this._discardMode) el.classList.add('discard-mode');

      // 显示时机标签
      const timingLabel = this._getTimingLabel(card.function_time);
      const needTarget = card.target_selection > 0 ? '🎯' : '';

      el.innerHTML = `
        <div class="card-img" style="background-image:url('${card.image_path || ''}')"></div>
        <div class="card-timing">${timingLabel}</div>
        <div class="card-name">${card.name} ${needTarget}</div>
        <div class="card-desc">${card.description}</div>
        <button class="card-use-btn" title="使用卡牌">使用</button>
      `;

      // 使用按钮
      el.querySelector('.card-use-btn')?.addEventListener('click', (ev) => {
        ev.stopPropagation();  // 防止触发卡牌选中
        console.log('[CardUseBtn] click, card=', card.name, 'id=', card.id);
        CardUI.selectedIdx = idx;
        if (window._gamePageUseCard) {
          window._gamePageUseCard(card);
        }
      });

      // 左键：第一击选中，再击同一张 → 使用
      el.addEventListener('click', () => {
        console.log('[CardClick] idx=', idx, 'card=', card.name,
                    'discardMode=', this._discardMode,
                    'oldSelected=', this.selectedIdx);
        if (this._discardMode) {
          this._finishDiscard(idx);
          return;
        }
        if (this.selectedIdx === idx) {
          // 已经选中 → 再次点击 = 使用
          console.log('[CardClick] 再次点击同一张卡牌，触发使用');
          if (window._gamePageUseCard) {
            window._gamePageUseCard(card);
          }
          return;
        }
        // 选中这张卡
        const oldIdx = this.selectedIdx;
        this.selectedIdx = idx;
        console.log('[CardClick] newSelected=', this.selectedIdx);
        if (oldIdx >= 0) {
          const oldEl = this.container.querySelectorAll('.card-item')[oldIdx];
          if (oldEl) oldEl.classList.remove('selected');
        }
        el.classList.add('selected');
      });

      // 右键：主动弃牌
      el.addEventListener('contextmenu', (e) => {
        e.preventDefault();
        if (this._discardMode) {
          this._cancelNewCard();
        } else {
          this.discardCard(idx);
        }
      });

      this.container.appendChild(el);
    });
  },

  // 获取时机标签文本
  _getTimingLabel(ft) {
    // CardFunctionTime: ANYTIME=0, YOUR_TURN=1, BEFORE_ROLL=2, BEFORE_MOVE=3, AFTER_MOVE=4
    switch (ft) {
      case 0: return '任意';
      case 1: return '自己回合';
      case 2: return '掷骰前';
      case 3: return '移动前';
      case 4: return '移动后';
      default: return '';
    }
  },

  _matchesPhase(functionTime, phaseType) {
    switch (functionTime) {
      case 0: // ANYTIME
      case 1: // YOUR_TURN
        return true;
      case 2: // BEFORE_ROLL
      case 3: // BEFORE_MOVE
        return phaseType === 'before_roll';
      case 4: // AFTER_MOVE
        return phaseType === 'after_move';
      default:
        return false;
    }
  }
};
