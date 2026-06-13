// ============================================================
// 应用入口 — 页面路由 & 全局状态
// ============================================================
const App = {
  currentPage: 'home',
  userColor: -1,
  userName: 'Player',

  init() {
    // 初始化各页面
    HomePage.init();
    LobbyPage.init();
    // GamePage.init() 在 navigate('game') 时延迟初始化

    // 弹窗关闭
    document.getElementById('btn-close-modal').addEventListener('click', () => {
      document.getElementById('modal-overlay').classList.remove('visible');
      document.getElementById('result-modal').classList.remove('visible');
      this.navigate('lobby');
    });

    // 默认显示主页
    this.navigate('home');
  },

  navigate(page) {
    // 隐藏当前页
    const old = document.querySelector('.page.active');
    if (old) old.classList.remove('active');

    // 显示目标页
    const target = document.getElementById(`page-${page}`);
    if (target) target.classList.add('active');

    // 每次进入 lobby 刷新 UI 权限（房主/玩家）
    if (page === 'lobby') {
      LobbyPage.init();
    }

    // 延迟初始化游戏页
    if (page === 'game' && this.currentPage !== 'game') {
      GamePage.init();
    }

    this.currentPage = page;
  }
};

// 启动
document.addEventListener('DOMContentLoaded', () => {
  App.init();
});
