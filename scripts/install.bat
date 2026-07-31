@echo off
setlocal

REM Optional install (default workflow does NOT install).
cd /d "%~dp0..\.."
set "PROJECTROOT=%CD%"

set "CMAKE=D:\win10\cmake-4.3.2-windows-x86_64\bin\cmake.exe"
set "BUILD_DIR=%PROJECTROOT%\build_msvc"
set "INSTALL_DIR=%PROJECTROOT%\install"

if not exist "%BUILD_DIR%" (
    echo [error] Build dir not found. Run build.bat first.
    exit /b 1
)

"%CMAKE%" --install "%BUILD_DIR%" --prefix "%INSTALL_DIR%"
if errorlevel 1 exit /b 1

echo [done] Installed to %INSTALL_DIR%
endlocal
