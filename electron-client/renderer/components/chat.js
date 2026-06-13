// ============================================================
// 聊天组件
// ============================================================
const ChatUI = {
  lists: {},  // {listId: DOM element}

  init(listId) {
    this.lists[listId] = document.getElementById(listId);
  },

  addMessage(listId, name, color, text) {
    const list = this.lists[listId] || document.getElementById(listId);
    if (!list) return;

    const colors = { 0: '#ff4d4d', 1: '#4d4dff', 2: '#33cc33', 3: '#cccc00' };
    const c = colors[color] || '#ccc';

    const line = document.createElement('div');
    line.className = 'chat-line';
    line.innerHTML = `<span class="name" style="color:${c}">${name}:</span> ${text}`;
    list.appendChild(line);
    list.scrollTop = list.scrollHeight;
  },

  addSystem(listId, text) {
    const list = this.lists[listId] || document.getElementById(listId);
    if (!list) return;

    const line = document.createElement('div');
    line.className = 'chat-line';
    line.innerHTML = `<span style="color:#888">${text}</span>`;
    list.appendChild(line);
    list.scrollTop = list.scrollHeight;
  }
};
