@echo off
setlocal
if not exist "%~dp0..\build\LOGL.exe" (
    echo Renderer not built. Run code\build.bat first.
    exit /b 1
)
pushd "%~dp0..\build"
if errorlevel 1 exit /b 1
LOGL.exe
set "runResult=%errorlevel%"
popd
exit /b %runResult%
