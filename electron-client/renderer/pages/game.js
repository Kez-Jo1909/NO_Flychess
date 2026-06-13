// ============================================================
// 游戏页面 — 棋盘交互 + 卡牌 + 骰子 + 聊天
// ============================================================
const GamePage = {
  myColor: -1,
  myTurn: false,
  diceRolled: false,
  cardPhase: false,
  cardData: null,  // 缓存的卡牌 JSON 配置

  async init() {
    this.myColor = App.userColor;

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

    // 掷骰子按钮
    document.getElementById('btn-roll-dice').addEventListener('click', () => {
      WS.send({ type: 'rolldice' });
      DiceUI.disableRoll('掷骰中...');
    });

    // 结束出牌按钮
    const finishCardBtn = document.getElementById('btn-finish-card');
    finishCardBtn.addEventListener('click', () => {
      WS.send({ type: 'finish_use_card', color: this.myColor });
      finishCardBtn.disabled = true;
      this.cardPhase = false;
      CardUI.clear();
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
      this.myTurn = true;
      this.diceRolled = false;
      DiceUI.enableRoll(() => {
        WS.send({ type: 'rolldice' });
        DiceUI.disableRoll('掷骰中...');
      });
      // 不清零骰子——保留上次掷出的数字供查看
      document.getElementById('btn-finish-card').disabled = true;
      ChatUI.addSystem('game-chat', '轮到你了，请掷骰子');
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
      DiceUI.disableRoll('选择棋子');

      if (msg.player_color !== this.myColor) {
        ChatUI.addSystem('game-chat',
          `${msg.name} 掷出了 ${msg.dice_result}`);
      }
    });

    WS.on('get_card', (msg) => {
      const cardId = msg.card_id;
      if (this.cardData) {
        const info = this.cardData.find(c => c.id === cardId);
        if (info) {
          CardUI.addCard(info);
          ChatUI.addSystem('game-chat', `获得卡牌: ${info.name}`);
        }
      }
    });

    WS.on('to_use_card', () => {
      this.cardPhase = true;
      document.getElementById('btn-finish-card').disabled = false;
      ChatUI.addSystem('game-chat', '出牌阶段（选择卡牌使用，或跳过）');
    });

    WS.on('all_piece_info', (msg) => {
      console.log('[Game] all_piece_info 收到, pieces=', msg.pieces ? msg.pieces.length : 0,
                  'sample=', JSON.stringify((msg.pieces || []).slice(0, 2)));
      Board.setPieces(msg.pieces);
    });

    WS.on('no_avialable_piece', (msg) => {
      ChatUI.addSystem('game-chat', '没有可用的棋子，跳过回合');
      this.myTurn = false;
    });

    WS.on('someone_finished', (msg) => {
      const names = ['红', '蓝', '绿', '黄'];
      ChatUI.addSystem('game-chat',
        `${names[msg.color] || msg.color} 方已完成游戏！`);
    });

    WS.on('all_finished', (msg) => {
      const names = ['红', '蓝', '绿', '黄'];
      const ranking = msg.rank.map((r, i) =>
        `第 ${i + 1} 名：${names[r.color] || r.color} 方`);
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
    if (playerId !== this.myColor) return;  // 不能操作别人的棋子

    if (this.cardPhase) {
      this._useCardOnPiece(playerId, pieceId);
      return;
    }

    if (this.diceRolled) {
      // 发送选择棋子
      WS.send({
        type: 'choose_chess_piece',
        id: pieceId,
        color: this.myColor
      });
      this.diceRolled = false;
      this.myTurn = false;
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

    // 如果没卡了，自动结束出牌
    if (CardUI.cards.length === 0) {
      document.getElementById('btn-finish-card').click();
    }
  }
};

// 卡牌双击使用（仅无目标卡牌 target_selection == 0）
document.addEventListener('DOMContentLoaded', () => {
  document.getElementById('card-hand')?.addEventListener('dblclick', (e) => {
    const cardEl = e.target.closest('.card-item');
    if (!cardEl || !GamePage.cardPhase) return;

    const card = CardUI.getSelected();
    if (!card) return;

    // 只有无需目标的卡牌才能双击直接使用
    const needTarget = card.target_selection && card.target_selection > 0;
    if (needTarget) {
      ChatUI.addSystem('game-chat', `【${card.name}】需要选择目标，请点击棋盘上的棋子`);
      return;
    }

    WS.send({
      type: 'use_card',
      card_id: card.id,
      target_id: -1,
      piece_id: -1
    });

    CardUI.removeCard(CardUI.selectedIdx);
    if (CardUI.cards.length === 0) {
      document.getElementById('btn-finish-card').click();
    }
  });
});
