@echo off
chcp 65001 >nul

:: ============================================================
:: NO_Flychess Release 打包
:: 双击运行，或: release.bat 0.5.0
:: ============================================================
set ROOT=%~dp0..
set VERSION=%~1
if "%VERSION%"=="" set VERSION=0.0.0

echo ========================================
echo  NO_Flychess Release 打包  v%VERSION%
echo ========================================

:: ---- 1. 编译 C++ 服务端 ----
echo.
echo [1/4] 编译 FlychessServer...
cd /d "%ROOT%\build"
if errorlevel 1 (
    echo [错误] build 目录不存在，请先运行 cmake
    pause
    exit /b 1
)
cmake --build . --config Release || goto :err
echo [1/4] OK

:: ---- 2. 复制服务端 ----
echo.
echo [2/4] 复制到 server-bin...
if exist "%ROOT%\electron-client\server-bin" rd /s /q "%ROOT%\electron-client\server-bin"
mkdir "%ROOT%\electron-client\server-bin\config"

set EXE=%ROOT%\build\server\FlychessServer.exe
if not exist "%EXE%" set EXE=%ROOT%\build\server\Release\FlychessServer.exe
if not exist "%EXE%" (
    echo [错误] 找不到 FlychessServer.exe
    pause
    exit /b 1
)
copy /y "%EXE%" "%ROOT%\electron-client\server-bin\" >nul

set CFG=%ROOT%\build\server\config
if not exist "%CFG%" set CFG=%ROOT%\config
xcopy /e /y /q "%CFG%\*" "%ROOT%\electron-client\server-bin\config\" >nul
echo [2/4] OK

:: ---- 3. 版本号 ----
echo.
echo [3/4] 更新版本号...
cd /d "%ROOT%\electron-client"
echo const p=require('./package.json');p.version='%VERSION%';require('fs').writeFileSync('package.json',JSON.stringify(p,null,2)+'\n'); > _ver.js
node _ver.js
del _ver.js
echo [3/4] OK

:: ---- 4. 打包 Electron ----
echo.
echo [4/4] 打包 Electron...
cd /d "%ROOT%\electron-client"
if not exist "node_modules" call npm install
call npm run build || goto :err

echo.
echo ========================================
echo  OK!
echo  输出: %ROOT%\electron-client\dist\
echo ========================================
dir "%ROOT%\electron-client\dist\*.exe" 2>nul
pause
exit /b 0

:err
echo.
echo ========================================
echo  打包失败!
echo ========================================
pause
exit /b 1
