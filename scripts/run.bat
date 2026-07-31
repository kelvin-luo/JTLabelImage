@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0..\.."
set "PROJECTROOT=%CD%"

set "QT_ROOT=D:\Qt6"
set "OPENCV_DIR=D:\win10\opencv4130\build"
set "RELEASE_DIR=%PROJECTROOT%\msvc_release"

set "QT_PREFIX="
if exist "%QT_ROOT%\msvc2022_64" set "QT_PREFIX=%QT_ROOT%\msvc2022_64"
for /d %%V in ("%QT_ROOT%\6.*") do (
    if exist "%%V\msvc2022_64" set "QT_PREFIX=%%V\msvc2022_64"
)

set "OCV_BIN="
if exist "%OPENCV_DIR%\x64\vc17\bin" set "OCV_BIN=%OPENCV_DIR%\x64\vc17\bin"
if exist "%OPENCV_DIR%\x64\vc16\bin" set "OCV_BIN=%OPENCV_DIR%\x64\vc16\bin"

set "PATH=%QT_PREFIX%\bin;%OCV_BIN%;%PATH%"

if not exist "%RELEASE_DIR%\JTLabelImage.exe" (
    echo [error] %RELEASE_DIR%\JTLabelImage.exe not found. Please run build.bat first.
    exit /b 1
)

if not exist "%RELEASE_DIR%\models" mkdir "%RELEASE_DIR%\models"
if not exist "%RELEASE_DIR%\input"  mkdir "%RELEASE_DIR%\input"
if not exist "%RELEASE_DIR%\output" mkdir "%RELEASE_DIR%\output"

start "" "%RELEASE_DIR%\JTLabelImage.exe"
endlocal
