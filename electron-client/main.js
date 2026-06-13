const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');
const { spawn } = require('child_process');

let mainWindow;
let serverProcess = null;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1200,
    height: 850,
    minWidth: 1000,
    minHeight: 750,
    title: 'NO_Flychess',
    backgroundColor: '#1a1a2e',
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false
    }
  });

  mainWindow.loadFile(path.join(__dirname, 'renderer', 'index.html'));
  mainWindow.setMenuBarVisibility(false);

  if (process.argv.includes('--dev')) {
    mainWindow.webContents.openDevTools();
  }

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

// ============================================================
// IPC: 读取配置文件
// ============================================================
ipcMain.handle('load-config', async (event, filename) => {
  const configPath = path.join(__dirname, '..', 'config', filename);
  try {
    const data = fs.readFileSync(configPath, 'utf-8');
    return { success: true, data: JSON.parse(data) };
  } catch (e) {
    console.error(`[Main] 读取配置失败: ${configPath}`, e.message);
    return { success: false, error: e.message };
  }
});

// ============================================================
// IPC: 启动/停止本地服务端
// ============================================================
ipcMain.handle('start-server', async () => {
  if (serverProcess) {
    return { success: false, error: '服务端已在运行' };
  }

  // 查找服务端可执行文件
  const serverExe = path.join(__dirname, '..', 'build', 'server', 'FlychessServer.exe');
  if (!fs.existsSync(serverExe)) {
    return { success: false, error: `找不到服务端: ${serverExe}` };
  }

  try {
    serverProcess = spawn(serverExe, [], {
      cwd: path.join(__dirname, '..', 'build', 'server'),
      stdio: 'pipe'
    });

    serverProcess.stdout.on('data', (data) => {
      console.log(`[Server] ${data.toString().trim()}`);
      if (mainWindow) {
        mainWindow.webContents.send('server-log', data.toString().trim());
      }
    });

    serverProcess.stderr.on('data', (data) => {
      console.error(`[Server Err] ${data.toString().trim()}`);
    });

    serverProcess.on('close', (code) => {
      console.log(`[Server] 进程退出, code=${code}`);
      serverProcess = null;
      if (mainWindow) {
        mainWindow.webContents.send('server-stopped', code);
      }
    });

    console.log('[Main] 服务端已启动');
    return { success: true };
  } catch (e) {
    console.error('[Main] 启动服务端失败:', e.message);
    return { success: false, error: e.message };
  }
});

ipcMain.handle('stop-server', async () => {
  if (!serverProcess) {
    return { success: false, error: '服务端未在运行' };
  }
  serverProcess.kill();
  serverProcess = null;
  return { success: true };
});

ipcMain.handle('is-server-running', async () => {
  return serverProcess !== null;
});

// ============================================================
// 应用生命周期
// ============================================================
app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (serverProcess) {
    serverProcess.kill();
    serverProcess = null;
  }
  app.quit();
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
