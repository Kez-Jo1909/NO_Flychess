// ============================================================
// 游戏页面 — 棋盘交互 + 卡牌 + 骰子 + 聊天
// ============================================================
const GamePage = {
  myColor: -1,
  myTurn: false,
  diceRolled: false,
  cardPhase: false,       // 出牌阶段（ROLLING 或 CARDING 时均可出牌）
  cardPhaseType: null,    // 'before_roll' | 'after_move' | null
  cardData: null,         // 缓存的卡牌 JSON 配置
  _settlementTimer: null, // 结算倒计时句柄（避免重复触发）

  async init() {
    this.myColor = App.userColor;

    // 重置状态
    this.myTurn = false;
    this.diceRolled = false;
    this.cardPhase = false;
    this.cardPhaseType = null;
    CardUI.clear();

    // 通过 IPC 加载卡牌配置
    try {
      const result = await window.electronAPI.loadConfig('card.json');
      if (result.success) {
        this.cardData = result.data;
        console.log('[Game] 卡牌配置:', this.cardData.length, '张');
      }
    } catch (e) {
      console.error('[Game] 卡牌配置加载失败:', e);
    }

    // 初始化棋盘
    await Board.init('chess-canvas');
    Board.onPieceClick = (playerId, pieceId) => this._onPieceClick(playerId, pieceId);

    // 初始化卡牌 UI
    CardUI.init('card-hand');

    // 初始化骰子
    DiceUI.init('dice-display', 'btn-roll-dice');

    // 结束出牌按钮（clone + replace 避免重复绑定）
    const oldFinishBtn = document.getElementById('btn-finish-card');
    const newFinishBtn = oldFinishBtn.cloneNode(true);
    oldFinishBtn.parentNode.replaceChild(newFinishBtn, oldFinishBtn);
    newFinishBtn.addEventListener('click', () => {
      WS.send({ type: 'finish_use_card', color: this.myColor });
      newFinishBtn.disabled = true;
      this.cardPhase = false;
      this.cardPhaseType = null;
    });

    // 游戏聊天（同样避免重复绑定）
    const oldChatBtn = document.getElementById('btn-game-chat-send');
    const newChatBtn = oldChatBtn.cloneNode(true);
    oldChatBtn.parentNode.replaceChild(newChatBtn, oldChatBtn);
    newChatBtn.addEventListener('click', () => {
      const input = document.getElementById('game-chat-input');
      const msg = input.value.trim();
      if (msg) {
        WS.send({ type: 'chat_message', message: msg });
        input.value = '';
      }
    });

    // ---- 服务端消息监听（仅首次注册，避免重复 handler）----
    if (!this._messagesBound) {
      this._messagesBound = true;
      this._bindMessages();
    }
  },

  _bindMessages() {
    WS.on('to_roll_dice', () => {
      console.log('[State] to_roll_dice — 进入掷骰阶段, cardPhase=', this.cardPhase,
                  'cards=', CardUI.cards.length);
      this.myTurn = true;
      this.diceRolled = false;
      DiceUI.enableRoll(() => {
        WS.send({ type: 'rolldice' });
        DiceUI.disableRoll('掷骰中...');
      });
      document.getElementById('btn-finish-card').disabled = true;
      this.cardPhase = false;
      this.cardPhaseType = null;
      ChatUI.addSystem('game-chat', '轮到你了，请掷骰子（或使用掷骰前卡牌）');
    });

    WS.on('to_roll_dice_broadcast', (msg) => {
      if (msg.color !== this.myColor) {
        const names = ['红', '蓝', '绿', '黄'];
        ChatUI.addSystem('game-chat', `等待 ${names[msg.color] || msg.color} 方掷骰子`);
      }
    });

    WS.on('dice_result', (msg) => {
      DiceUI.setValue(msg.dice_result);
      this.diceRolled = true;
      this.cardPhase = false;
      this.cardPhaseType = null;
      document.getElementById('btn-finish-card').disabled = true;

      if (msg.player_color !== this.myColor) {
        ChatUI.addSystem('game-chat',
          `${msg.name} 掷出了 ${msg.dice_result}`);
      } else {
        this.myTurn = true;
        DiceUI.disableRoll('选择棋子');

        // 自动移动：剩余唯一可动棋子时自动选择
        const myUnfinished = (Board.pieces || []).filter(
          p => p.player_id === this.myColor && p.position !== -2
        );
        if (myUnfinished.length === 1) {
          const piece = myUnfinished[0];
          const canMove = piece.position >= 0 || msg.dice_result === 6;
          if (canMove) {
            console.log('[Auto] 唯一剩余棋子 id=', piece.id,
                        'pos=', piece.position, 'dice=', msg.dice_result, '自动选择');
            setTimeout(() => {
              if (this.myTurn && this.diceRolled) {
                this._onPieceClick(this.myColor, piece.id);
              }
            }, 400);
          }
        }
      }
    });

    WS.on('get_card', (msg) => {
      const cardId = msg.card_id;
      if (this.cardData) {
        const info = this.cardData.find(c => c.id === cardId);
        if (info) {
          const added = CardUI.addCard(info);
          if (added) {
            ChatUI.addSystem('game-chat', `获得卡牌: ${info.name}`);
          }
        }
      }
    });

    WS.on('to_use_card', (msg) => {
      console.log('[State] to_use_card phase=', msg.phase,
                  'cards=', CardUI.cards.length,
                  'myTurn=', this.myTurn, 'diceRolled=', this.diceRolled);
      if (msg.phase !== 'before_roll' && CardUI.cards.length === 0) {
        console.log('[State] 无手牌，自动跳过出牌阶段');
        WS.send({ type: 'finish_use_card', color: this.myColor });
        this.cardPhase = false;
        this.cardPhaseType = null;
        return;
      }
      this.cardPhase = true;
      this.cardPhaseType = msg.phase || 'after_move';
      document.getElementById('btn-finish-card').disabled = false;

      if (this.cardPhaseType === 'before_roll') {
        ChatUI.addSystem('game-chat', '掷骰前阶段（可使用卡牌，或直接掷骰子）');
      } else {
        ChatUI.addSystem('game-chat', '出牌阶段（选择卡牌使用，或跳过）');
      }
    });

    WS.on('all_piece_info', (msg) => {
      console.log('[Game] all_piece_info 收到, pieces=', msg.pieces ? msg.pieces.length : 0,
                  'sample=', JSON.stringify((msg.pieces || []).slice(0, 2)));
      Board.setPieces(msg.pieces);
    });

    WS.on('no_avialable_piece', (msg) => {
      ChatUI.addSystem('game-chat', '没有可用的棋子，跳过回合');
      this.myTurn = false;
      this.cardPhase = false;
      this.cardPhaseType = null;
      document.getElementById('btn-finish-card').disabled = true;
    });

    WS.on('someone_finished', (msg) => {
      const names = ['红', '蓝', '绿', '黄'];
      ChatUI.addSystem('game-chat',
        `${names[msg.color] || msg.color} 方已完成游戏！`);
    });

    // ============================================================
    // all_finished — 结算弹窗 + 自动返回房间
    // ============================================================
    WS.on('all_finished', (msg) => {
      console.log('[Settlement] all_finished 触发, rank=', JSON.stringify(msg.rank));

      // 防御：清除上一次结算的定时器（如果存在）
      if (this._settlementTimer) {
        clearInterval(this._settlementTimer);
        this._settlementTimer = null;
      }

      const names = ['红', '蓝', '绿', '黄'];
      const medals = ['【冠军】', '【亚军】', '【季军】', '【第 4 名】'];

      const rankingHtml = msg.rank.map((r, i) => {
        const name = r.name || (names[r.color] || r.color + '方');
        const medal = medals[i] || `【第 ${i + 1} 名】`;
        return `<div class="rank-line rank-${i}">
          <span class="rank-medal">${medal}</span>
          <span class="rank-name">${name}</span>
        </div>`;
      }).join('');

      const rankingTextEl = document.getElementById('ranking-text');
      rankingTextEl.innerHTML = rankingHtml;

      // 追加倒计时元素
      const oldCountdown = document.getElementById('countdown-timer');
      if (oldCountdown) oldCountdown.remove();

      const countdownEl = document.createElement('div');
      countdownEl.id = 'countdown-timer';
      countdownEl.style.cssText =
        'margin-top:16px;font-size:14px;color:var(--text-secondary);';
      rankingTextEl.appendChild(countdownEl);

      // 同步按钮与倒计时
      const closeBtn = document.getElementById('btn-close-modal');
      let countdown = 5;
      const updateUI = () => {
        countdownEl.textContent = `${countdown} 秒后自动返回房间...`;
        closeBtn.textContent = `回到房间（${countdown} 秒后自动返回）`;
      };
      updateUI();

      // 显示结算弹窗（必须移除 hidden，否则 display:none!important 优先级高于 visible）
      document.getElementById('modal-overlay').classList.remove('hidden');
      document.getElementById('modal-overlay').classList.add('visible');
      document.getElementById('result-modal').classList.remove('hidden');
      document.getElementById('result-modal').classList.add('visible');
      console.log('[Settlement] 弹窗已显示');

      // 倒计时自动返回
      this._settlementTimer = setInterval(() => {
        countdown--;
        if (countdown <= 0) {
          console.log('[Settlement] 倒计时结束，自动返回房间');
          clearInterval(this._settlementTimer);
          this._settlementTimer = null;
          closeBtn.click();
        } else {
          updateUI();
        }
      }, 1000);

      // 手动点击"回到房间"时清除定时器
      closeBtn.addEventListener('click', () => {
        if (this._settlementTimer) {
          console.log('[Settlement] 手动返回，清除定时器');
          clearInterval(this._settlementTimer);
          this._settlementTimer = null;
        }
      }, { once: true });
    });

    WS.on('chat_message_broadcast', (msg) => {
      ChatUI.addMessage('game-chat', msg.name, msg.color, msg.chat_msg);
    });
  },

  // 棋子点击回调
  _onPieceClick(playerId, pieceId) {
    if (!this.myTurn) return;

    if (this.cardPhase) {
      if (this.cardPhaseType === 'after_move') {
        this._useCardOnPiece(playerId, pieceId);
        return;
      }
    }

    if (this.diceRolled && playerId === this.myColor) {
      WS.send({
        type: 'choose_chess_piece',
        id: pieceId,
        color: this.myColor
      });
      this.diceRolled = false;
      this.myTurn = false;
      this.cardPhase = false;
      this.cardPhaseType = null;
      DiceUI.disableRoll('已选择');
    }
  },

  // 在有目标选择的卡牌上使用棋子
  _useCardOnPiece(playerId, pieceId) {
    const card = CardUI.getSelected();
    if (!card) {
      ChatUI.addSystem('game-chat', '请先选择一张卡牌');
      return;
    }

    WS.send({
      type: 'use_card',
      card_id: card.id,
      target_id: playerId,
      piece_id: pieceId
    });

    CardUI.removeCard(CardUI.selectedIdx);

    if (CardUI.cards.length === 0 && GamePage.cardPhaseType === 'after_move') {
      document.getElementById('btn-finish-card').click();
    }
  }
};

// 暴露给 card.js：使用卡牌的核心逻辑
window._gamePageUseCard = function(card) {
  console.log('[UseCard] card=', card.name, 'id=', card.id,
              'cardPhase=', GamePage.cardPhase,
              'phaseType=', GamePage.cardPhaseType);

  if (!GamePage.cardPhase) {
    ChatUI.addSystem('game-chat', '当前不是出牌阶段');
    return;
  }

  const needTarget = card.target_selection && card.target_selection > 0;
  if (needTarget) {
    ChatUI.addSystem('game-chat', `【${card.name}】需要选择目标，请点击棋盘上的棋子`);
    return;
  }

  if (GamePage.cardPhaseType === 'before_roll') {
    DiceUI.disableRoll('卡牌生效中...');
  }

  console.log('[UseCard] 发送 use_card, id=', card.id);
  WS.send({
    type: 'use_card',
    card_id: card.id,
    target_id: -1,
    piece_id: -1
  });

  CardUI.removeCard(CardUI.selectedIdx);

  if (CardUI.cards.length === 0 && GamePage.cardPhaseType === 'after_move') {
    console.log('[UseCard] 手牌用尽，自动结束出牌');
    document.getElementById('btn-finish-card').click();
  }
};

// 兼容：双击仍然可用
document.addEventListener('DOMContentLoaded', () => {
  document.getElementById('card-hand')?.addEventListener('dblclick', (e) => {
    const cardEl = e.target.closest('.card-item');
    if (!cardEl) return;
    const card = CardUI.getSelected();
    if (card) window._gamePageUseCard(card);
  });
});
