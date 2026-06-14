@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

:: ============================================================
:: NO_Flychess Release 打包（双击运行）
:: 用法: release.bat [版本号]
:: ============================================================
set ROOT=%~dp0..
set BUILD_DIR=%ROOT%\build
set EXE=%BUILD_DIR%\server\FlychessServer.exe
set CONFIG_SRC=%BUILD_DIR%\server\config
set SERVER_BIN=%ROOT%\electron-client\server-bin
set ELECTRON=%ROOT%\electron-client

if "%~1"=="" (set VERSION=0.0.0) else (set VERSION=%~1)

echo ========================================
echo  NO_Flychess Release 打包  v%VERSION%
echo ========================================

:: ---- 1. 编译 ----
echo.
echo [1/4] 编译 FlychessServer (Release)...
cd /d "%BUILD_DIR%"
cmake --build . --config Release
if errorlevel 1 goto :err
echo [1/4] OK

:: ---- 2. 复制 ----
echo.
echo [2/4] 复制到 server-bin...
if exist "%SERVER_BIN%" rd /s /q "%SERVER_BIN%"
mkdir "%SERVER_BIN%\config"

if not exist "%EXE%" (
    echo [错误] 找不到 %EXE%
    goto :err
)
copy /y "%EXE%" "%SERVER_BIN%\" >nul
if exist "%CONFIG_SRC%" (
    xcopy /e /y /q "%CONFIG_SRC%\*" "%SERVER_BIN%\config\" >nul
) else if exist "%ROOT%\config" (
    xcopy /e /y /q "%ROOT%\config\*" "%SERVER_BIN%\config\" >nul
)
echo [2/4] OK

:: ---- 3. 版本号 ----
echo.
echo [3/4] 版本号: %VERSION%
cd /d "%ELECTRON%"
node -e "const p=require('./package.json');p.version='%VERSION%';require('fs').writeFileSync('package.json',JSON.stringify(p,null,2)+'\n');"

:: ---- 4. 打包 Electron ----
echo.
echo [4/4] 打包 Electron...
if not exist "%ELECTRON%\node_modules" (
    echo  安装依赖...
    cd /d "%ELECTRON%"
    call npm install
    if errorlevel 1 goto :err
)
cd /d "%ELECTRON%"
call npm run build
if errorlevel 1 goto :err

:: ---- OK ----
echo.
echo ========================================
echo  OK!  输出: %ELECTRON%\dist\
echo ========================================
dir "%ELECTRON%\dist\*.exe" 2>nul || dir "%ELECTRON%\dist\"
goto :end

:err
echo.
echo ========================================
echo  打包失败! 按任意键退出...
echo ========================================
pause
exit /b 1

:end
endlocal
pause
