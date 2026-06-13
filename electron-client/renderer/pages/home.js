// ============================================================
// 主页 — 连接服务器 / 创建房间
// ============================================================
const HomePage = {
  _listenersSetup: false,

  init() {
    const btnConnect = document.getElementById('btn-connect');
    const btnHost = document.getElementById('btn-host');
    const urlInput = document.getElementById('server-url');
    const nameInput = document.getElementById('player-name');

    // 全局消息监听（只注册一次）
    if (!this._listenersSetup) {
      WS.on('register_ret', (msg) => {
        const color = parseInt(msg.color);
        App.userColor = color;
        console.log('[Home] 注册成功, color=', color);
        setTimeout(() => App.navigate('lobby'), 50);
      });

      WS.on('url', (msg) => {
        document.getElementById('lobby-url').textContent = msg.url;
      });

      WS.on('disconnected', () => {
        ChatUI.addSystem('chat-list', '与服务器的连接断开');
        btnConnect.disabled = false;
        btnConnect.textContent = '连接服务器';
        btnHost.disabled = false;
        btnHost.textContent = '创建房间（本机 host）';
      });

      this._listenersSetup = true;
    }

    // 连接已有服务器
    btnConnect.addEventListener('click', () => this._doConnect(false));

    // 创建房间：启动本地服务端 + 连接
    btnHost.addEventListener('click', async () => {
      btnHost.disabled = true;
      btnHost.textContent = '启动服务端中...';

      const result = await window.electronAPI.startServer();
      if (!result.success) {
        ChatUI.addSystem('chat-list', `服务端启动失败: ${result.error}`);
        btnHost.disabled = false;
        btnHost.textContent = '创建房间（本机 host）';
        return;
      }

      ChatUI.addSystem('chat-list', '本地服务端已启动');
      // 等 500ms 让服务端就绪
      await new Promise(r => setTimeout(r, 500));
      this._doConnect(true);
    });
  },

  _doConnect(isHost) {
    const url = document.getElementById('server-url').value.trim();
    const name = document.getElementById('player-name').value.trim() || 'Player';

    if (!url) return;
    if (WS.isConnected()) {
      ChatUI.addSystem('chat-list', '已经连接了');
      return;
    }

    document.getElementById('btn-connect').disabled = true;
    document.getElementById('btn-connect').textContent = '连接中...';
    document.getElementById('btn-host').disabled = true;

    WS.onConnect(() => {
      WS.send({ type: 'userInfo', name: name });
      ChatUI.addSystem('chat-list', isHost ? '房间已创建' : '已连接到服务器');
      document.getElementById('btn-connect').textContent = '已连接';
    });

    WS.connect(url);
  }
};
