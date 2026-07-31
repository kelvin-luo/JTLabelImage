@echo off
setlocal enabledelayedexpansion

REM scripts/ -> code/ -> PROJECT_DIR
cd /d "%~dp0..\.."
set "PROJECTROOT=%CD%"
cd /d "%~dp0.."

REM ===== Toolchain paths =====
set "CMAKE=D:\win10\cmake-4.3.2-windows-x86_64\bin\cmake.exe"
set "NINJA=D:\win10\ninja.exe"
set "QT_ROOT=D:\Qt6"
set "OPENCV_DIR=D:\win10\opencv4130\build"
set "VS_VCVARS="
for %%E in (Enterprise Professional Community BuildTools) do (
    if not defined VS_VCVARS (
        if exist "C:\Program Files\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars64.bat" (
            set "VS_VCVARS=C:\Program Files\Microsoft Visual Studio\2022\%%E\VC\Auxiliary\Build\vcvars64.bat"
        )
    )
)

set "BUILD_DIR=%PROJECTROOT%\build_msvc"
set "RELEASE_DIR=%PROJECTROOT%\msvc_release"

REM ===== Auto-detect Qt6 msvc2022_64 prefix =====
set "QT_PREFIX="
if exist "%QT_ROOT%\msvc2022_64" set "QT_PREFIX=%QT_ROOT%\msvc2022_64"
for /d %%V in ("%QT_ROOT%\6.*") do (
    if exist "%%V\msvc2022_64" set "QT_PREFIX=%%V\msvc2022_64"
)
if "%QT_PREFIX%"=="" set "QT_PREFIX=%QT_ROOT%"
echo [info] PROJECTROOT: %PROJECTROOT%
echo [info] Qt prefix  : %QT_PREFIX%
echo [info] OpenCV     : %OPENCV_DIR%

if not exist "%VS_VCVARS%" (
    echo [error] Cannot find vcvars64.bat
    exit /b 1
)
call "%VS_VCVARS%" >nul

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%RELEASE_DIR%" mkdir "%RELEASE_DIR%"
if not exist "%RELEASE_DIR%\models" mkdir "%RELEASE_DIR%\models"
if not exist "%RELEASE_DIR%\input"  mkdir "%RELEASE_DIR%\input"
if not exist "%RELEASE_DIR%\output" mkdir "%RELEASE_DIR%\output"

"%CMAKE%" -S "%PROJECTROOT%\code" -B "%BUILD_DIR%" -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MAKE_PROGRAM="%NINJA%" ^
    -DCMAKE_C_COMPILER=cl ^
    -DCMAKE_CXX_COMPILER=cl ^
    -DCMAKE_PREFIX_PATH="%QT_PREFIX%;%OPENCV_DIR%" ^
    -DOpenCV_DIR="%OPENCV_DIR%"
if errorlevel 1 exit /b 1

"%CMAKE%" --build "%BUILD_DIR%" --parallel
if errorlevel 1 exit /b 1

echo.
echo [done] Output: %RELEASE_DIR%\JTLabelImage.exe
endlocal
