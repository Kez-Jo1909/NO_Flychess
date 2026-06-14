@echo off
setlocal

:: ============================================================
:: NO_Flychess Release Builder
:: Usage: release.bat [version]
:: ============================================================
set ROOT=%~dp0..
set VERSION=%~1
if "%VERSION%"=="" set VERSION=0.0.0

echo ========================================
echo  NO_Flychess Release Builder v%VERSION%
echo ========================================

:: ---- 1. Build C++ server ----
echo.
echo [1/4] Building FlychessServer...
cd /d "%ROOT%\build"
if errorlevel 1 (
    echo [ERROR] build dir not found, run cmake first
    pause
    exit /b 1
)
cmake --build . --config Release
if errorlevel 1 goto :fail
echo [1/4] OK

:: ---- 2. Copy server to package dir ----
echo.
echo [2/4] Copying to server-bin...
set DST=%ROOT%\electron-client\server-bin
if exist "%DST%" rd /s /q "%DST%"
mkdir "%DST%\config"

set EXE=%ROOT%\build\server\FlychessServer.exe
if not exist "%EXE%" set EXE=%ROOT%\build\server\Release\FlychessServer.exe
if not exist "%EXE%" (
    echo [ERROR] FlychessServer.exe not found
    pause
    exit /b 1
)
copy /y "%EXE%" "%DST%\" >nul

set CFG=%ROOT%\build\server\config
if not exist "%CFG%" set CFG=%ROOT%\config
xcopy /e /y /q "%CFG%\*" "%DST%\config\" >nul
echo [2/4] OK

:: ---- 3. Update version ----
echo.
echo [3/4] Updating version...
cd /d "%ROOT%\electron-client"
(
echo const p=require('./package.json'^);
echo p.version='%VERSION%'^;
echo require('fs'^).writeFileSync('package.json',JSON.stringify(p,null,2^)+'\n'^)^;
) > _ver.js
node _ver.js
del _ver.js
echo [3/4] OK

:: ---- 4. Package Electron ----
echo.
echo [4/4] Packaging Electron...
cd /d "%ROOT%\electron-client"
if not exist "node_modules" call npm install
call npm run build
if errorlevel 1 goto :fail

:: ---- Done ----
echo.
echo ========================================
echo  DONE! Output: %ROOT%\electron-client\dist\
echo ========================================
dir "%ROOT%\electron-client\dist\*.exe" 2>nul
pause
exit /b 0

:fail
echo.
echo ========================================
echo  BUILD FAILED!
echo ========================================
pause
exit /b 1
