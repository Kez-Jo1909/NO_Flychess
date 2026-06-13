// ============================================================
// WebSocket 客户端 — 单例，支持消息缓冲与回放
//
// 核心机制：先注册 listener，后连接。
// 如果消息到达时没有对应的 listener，消息会暂存在 buffer 中，
// 当 listener 被注册时立即回放。
// ============================================================
const WS = {
  _ws: null,
  _url: '',
  _handlers: {},
  _buffer: [],       // 未消费的消息缓冲 [{type, data}]
  _connected: false,
  _connectHandlers: [],  // connect 事件专用队列

  connect(url) {
    if (this._ws && this._ws.readyState === WebSocket.OPEN) {
      // 已连接，直接触发
      this._onConnected();
      return;
    }
    if (this._ws && this._ws.readyState === WebSocket.CONNECTING) return;

    this._url = url;
    this._ws = new WebSocket(url);

    this._ws.onopen = () => {
      console.log('[WS] 已连接:', url);
      this._onConnected();
    };

    this._ws.onmessage = (event) => {
      try {
        const msg = JSON.parse(event.data);
        const type = msg.type;
        console.log('[WS] 收到:', type, msg);
        this._emit(type, msg);
        this._emit('*', msg);
      } catch (e) {
        console.warn('[WS] 非 JSON 消息:', event.data);
      }
    };

    this._ws.onclose = () => {
      console.log('[WS] 断开');
      this._connected = false;
      this._emit('disconnected', {});
    };

    this._ws.onerror = (err) => {
      console.error('[WS] 错误:', err);
    };
  },

  send(obj) {
    if (this._ws && this._ws.readyState === WebSocket.OPEN) {
      this._ws.send(JSON.stringify(obj));
    } else {
      console.warn('[WS] 未连接，无法发送:', obj.type);
    }
  },

  // 注册消息监听器（注册时自动回放 buffer 中的同类型消息）
  on(type, callback) {
    if (!this._handlers[type]) this._handlers[type] = [];
    this._handlers[type].push(callback);

    // 回放 buffer 中该类型的消息
    if (this._buffer.length > 0) {
      const replay = this._buffer.filter(m => m.type === type);
      this._buffer = this._buffer.filter(m => m.type !== type);
      for (const m of replay) {
        try { callback(m.data); } catch (e) { console.error('[WS] replay error:', e); }
      }
    }
  },

  off(type, callback) {
    if (this._handlers[type]) {
      this._handlers[type] = this._handlers[type].filter(cb => cb !== callback);
    }
  },

  _emit(type, data) {
    if (this._handlers[type] && this._handlers[type].length > 0) {
      for (const cb of this._handlers[type]) {
        try { cb(data); } catch (e) { console.error('[WS] handler error:', e); }
      }
    } else {
      // 没有监听器 → 缓冲（排除高频消息避免内存泄漏）
      if (type !== '*' && type !== 'disconnected') {
        this._buffer.push({ type, data });
      }
    }
  },

  _onConnected() {
    this._connected = true;
    // 回放队列中的 connect 回调
    const cbs = [...this._connectHandlers];
    this._connectHandlers = [];
    for (const cb of cbs) cb();
  },

  // 注册连接成功回调（用于先注册再 connect 的场景）
  onConnect(callback) {
    if (this._connected) {
      callback();
    } else {
      this._connectHandlers.push(callback);
    }
  },

  isConnected() { return this._connected; },

  disconnect() {
    this._buffer = [];
    this._connectHandlers = [];
    if (this._ws) {
      this._ws.close();
      this._ws = null;
      this._connected = false;
    }
  }
};
