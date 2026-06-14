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
    // 掷骰按钮回调由 DiceUI.enableRoll() 接管，不在这里重复绑定

    // 结束出牌按钮
    const finishCardBtn = document.getElementById('btn-finish-card');
    finishCardBtn.addEventListener('click', () => {
      WS.send({ type: 'finish_use_card', color: this.myColor });
      finishCardBtn.disabled = true;
      this.cardPhase = false;
      this.cardPhaseType = null;
      // 不清理手牌——卡牌保留到下回合
    });

    // 游戏聊天
    document.getElementById('btn-game-chat-send').addEventListener('click', () => {
      const input = document.getElementById('game-chat-input');
      const msg = input.value.trim();
      if (msg) {
        WS.send({ type: 'chat_message', message: msg });
        input.value = '';
      }
    });

    // ---- 服务端消息监听 ----
    this._bindMessages();
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
      // 不清零骰子——保留上次掷出的数字供查看
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
      // 掷完骰子，before_roll 卡牌阶段结束
      this.cardPhase = false;
      this.cardPhaseType = null;
      document.getElementById('btn-finish-card').disabled = true;

      if (msg.player_color !== this.myColor) {
        // 别人的骰子结果
        ChatUI.addSystem('game-chat',
          `${msg.name} 掷出了 ${msg.dice_result}`);
      } else {
        // 我的骰子结果（含卡牌 6 触发）——确保可以选棋子
        this.myTurn = true;
        DiceUI.disableRoll('选择棋子');
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
          // 如果手牌已满，addCard 内部会触发弃牌流程并显示提示
        }
      }
    });

    WS.on('to_use_card', (msg) => {
      console.log('[State] to_use_card phase=', msg.phase,
                  'cards=', CardUI.cards.length,
                  'myTurn=', this.myTurn, 'diceRolled=', this.diceRolled);
      // 如果手牌为空且是 after_move 阶段，自动跳过出牌（直接发消息，不依赖按钮状态）
      if (msg.phase !== 'before_roll' && CardUI.cards.length === 0) {
        console.log('[State] 无手牌，自动跳过出牌阶段');
        WS.send({ type: 'finish_use_card', color: this.myColor });
        this.cardPhase = false;
        this.cardPhaseType = null;
        return;
      }
      this.cardPhase = true;
      this.cardPhaseType = msg.phase || 'after_move';  // 'before_roll' 或 'after_move'
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

    WS.on('all_finished', (msg) => {
      const names = ['红', '蓝', '绿', '黄'];
      const ranking = msg.rank.map((r, i) => {
        const name = r.name || (names[r.color] || r.color + '方');
        return `第 ${i + 1} 名：${name}`;
      });
      document.getElementById('ranking-text').innerHTML =
        ranking.join('<br>');
      document.getElementById('modal-overlay').classList.add('visible');
      document.getElementById('result-modal').classList.add('visible');
    });

    WS.on('chat_message_broadcast', (msg) => {
      ChatUI.addMessage('game-chat', msg.name, msg.color, msg.chat_msg);
    });
  },

  // 棋子点击回调
  _onPieceClick(playerId, pieceId) {
    if (!this.myTurn) return;

    if (this.cardPhase) {
      // after_move 出牌阶段：点击棋子 = 对目标棋子使用卡牌
      // before_roll 出牌阶段：点击棋子不触发卡牌目标选择（还没掷骰子）
      if (this.cardPhaseType === 'after_move') {
        this._useCardOnPiece(playerId, pieceId);
        return;
      }
      // before_roll：棋子点击穿透到下方的选棋子移动逻辑
    }

    if (this.diceRolled && playerId === this.myColor) {
      // 选棋子移动阶段：只能选自己的棋子
      WS.send({
        type: 'choose_chess_piece',
        id: pieceId,
        color: this.myColor
      });
      this.diceRolled = false;
      this.myTurn = false;
      this.cardPhase = false;
      this.cardPhaseType = null;
      // btn-finish-card 由 to_use_card 处理器管理，不在这里禁用
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

    // 只在 after_move 阶段：手牌用完后自动结束出牌
    // before_roll 阶段不自动结束（还需要掷骰子/选棋子）
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

  // 需要目标的卡牌不能直接使用
  const needTarget = card.target_selection && card.target_selection > 0;
  if (needTarget) {
    ChatUI.addSystem('game-chat', `【${card.name}】需要选择目标，请点击棋盘上的棋子`);
    return;
  }

  // 立刻禁用掷骰按钮，防止在服务器返回之前手快误掷
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

  // 只在 after_move 阶段：手牌用完后自动结束出牌
  // before_roll 阶段不自动结束（还需要掷骰子/选棋子）
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
