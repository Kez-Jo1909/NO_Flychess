// ============================================================
// 大厅 — 等待玩家，准备，聊天
// ============================================================
const LobbyPage = {
  _initialized: false,

  init() {
    if (this._initialized) {
      this._refreshHostUI();
      return;
    }
    this._initialized = true;

    // ---- DOM 事件（只绑定一次）----
    document.getElementById('cfg-player-count').addEventListener('change', function() {
      WS.send({ type: 'update_player_count', new_p_num: parseInt(this.value) });
    });
    document.getElementById('cfg-chess-count').addEventListener('change', function() {
      WS.send({ type: 'update_chess_count', new_c_num: parseInt(this.value) });
    });

    let ready = false;
    document.getElementById('btn-ready').addEventListener('click', () => {
      ready = !ready;
      if (ready) {
        WS.send({ type: 'get_prepared' });
        document.getElementById('btn-ready').textContent = '取消准备';
      } else {
        WS.send({ type: 'get_unprepared' });
        document.getElementById('btn-ready').textContent = '准备';
      }
    });

    document.getElementById('btn-start').addEventListener('click', () => {
      WS.send({ type: 'game_start' });
    });

    document.getElementById('btn-chat-send').addEventListener('click', () => {
      const input = document.getElementById('chat-input');
      const msg = input.value.trim();
      if (msg) {
        WS.send({ type: 'chat_message', message: msg });
        input.value = '';
      }
    });
    document.getElementById('chat-input').addEventListener('keydown', (e) => {
      if (e.key === 'Enter') document.getElementById('btn-chat-send').click();
    });

    // ---- WS 消息（注册后自动回放 buffer 中的消息）----
    WS.on('add_player_broadcast', (msg) => {
      ChatUI.addSystem('chat-list', `${msg.name} 加入了房间`);
    });

    WS.on('update_player_list', (msg) => {
      this._renderPlayerList(msg.players);
      // 同步人数
      const el = document.getElementById('lobby-player-count');
      const parts = el.textContent.split('/');
      el.textContent = `${msg.players.length}/${parts.length > 1 ? parts[1] : '4'}`;
    });

    // 玩家数上限变更 → 同步文字 + select
    WS.on('update_pc_ret', (msg) => {
      const el = document.getElementById('lobby-player-count');
      const parts = el.textContent.split('/');
      el.textContent = `${parts.length > 1 ? parts[0] : '0'}/${msg.new_count}`;
      document.getElementById('cfg-player-count').value = msg.new_count;
    });

    // 棋子数变更 → 同步文字 + select
    WS.on('update_cc_ret', (msg) => {
      document.getElementById('lobby-chess-count').textContent = msg.new_count;
      document.getElementById('cfg-chess-count').value = msg.new_count;
    });

    WS.on('chat_message_broadcast', (msg) => {
      ChatUI.addMessage('chat-list', msg.name, msg.color, msg.chat_msg);
    });

    WS.on('leave_room', (msg) => {
      ChatUI.addSystem('chat-list', `${msg.name} 离开了房间`);
    });

    WS.on('game_start', () => {
      App.navigate('game');
    });

    WS.on('not_ready', () => {
      ChatUI.addSystem('chat-list', '还有玩家未准备！');
    });

    WS.on('not_enough_players', () => {
      ChatUI.addSystem('chat-list', '玩家数量不足！');
    });

    // 初始 UI 状态
    this._refreshHostUI();
  },

  // 根据身份（房主/玩家）调整 UI 权限
  _refreshHostUI() {
    const isHost = App.userColor === 0;
    console.log('[Lobby] _refreshHostUI, userColor=', App.userColor, 'isHost=', isHost);

    // 房主：显示开始按钮，可修改设置
    const btnStart = document.getElementById('btn-start');
    if (isHost) {
      btnStart.classList.remove('hidden');
    } else {
      btnStart.classList.add('hidden');
    }

    // 非房主：禁用设置
    document.getElementById('cfg-player-count').disabled = !isHost;
    document.getElementById('cfg-chess-count').disabled = !isHost;
  },

  _renderPlayerList(players) {
    const container = document.getElementById('player-list');
    const colors = ['red', 'blue', 'green', 'yellow'];
    container.innerHTML = players.map(p => `
      <div class="player-item">
        <div class="player-dot dot-${colors[p.color] || 'red'}"></div>
        <div class="player-name">${p.name}</div>
        <div class="player-ready ${p.if_prepared ? 'ready' : 'not-ready'}">
          ${p.if_prepared ? '已准备' : '未准备'}
        </div>
      </div>
    `).join('');
  }
};
