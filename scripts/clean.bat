@echo off
setlocal

cd /d "%~dp0..\.."
set "PROJECTROOT=%CD%"

if exist "%PROJECTROOT%\build_msvc" rmdir /s /q "%PROJECTROOT%\build_msvc"
if exist "%PROJECTROOT%\msvc_release" (
    for %%f in ("%PROJECTROOT%\msvc_release\*.exe" "%PROJECTROOT%\msvc_release\*.dll" "%PROJECTROOT%\msvc_release\*.pdb" "%PROJECTROOT%\msvc_release\*.ilk") do del /q "%%f" 2>nul
    for /d %%d in ("%PROJECTROOT%\msvc_release\*") do (
        if /I not "%%~nxd"=="models" if /I not "%%~nxd"=="input" if /I not "%%~nxd"=="output" (
            rmdir /s /q "%%d" 2>nul
        )
    )
)

echo [done] cleaned build_msvc/ and msvc_release/ build artifacts under %PROJECTROOT%
echo [info] kept msvc_release\models, input, output
endlocal
