const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  platform: process.platform,
  isDev: process.argv.includes('--dev'),

  // 配置文件加载
  loadConfig: (filename) => ipcRenderer.invoke('load-config', filename),

  // 服务端控制
  startServer: () => ipcRenderer.invoke('start-server'),
  stopServer: () => ipcRenderer.invoke('stop-server'),
  isServerRunning: () => ipcRenderer.invoke('is-server-running'),

  // 服务端日志监听
  onServerLog: (callback) => {
    ipcRenderer.on('server-log', (event, msg) => callback(msg));
  },
  onServerStopped: (callback) => {
    ipcRenderer.on('server-stopped', (event, code) => callback(code));
  }
});
