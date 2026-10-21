@echo off
setlocal
set "rootDir=%~dp0.."
for %%I in ("%rootDir%") do set "rootDir=%%~fI"
set "configuration=%~1"
if not defined configuration set "configuration=debug"
if /I "%configuration%"=="debug" goto debug
if /I "%configuration%"=="release" goto release
if /I "%configuration%"=="clangd" goto compiler
echo Usage: build.bat [debug^|release^|clangd]
exit /b 2

:debug
set "modeFlags=/MTd /Od /Z7 /DLOGL_INTERNAL=1 /DLOGL_SLOW=1"
goto compiler
:release
set "modeFlags=/MT /O2 /Z7 /DLOGL_INTERNAL=1 /DLOGL_SLOW=1"

:compiler
where cl >nul 2>nul
if not errorlevel 1 goto dependencies
if defined LOGL_MSVC_SETUP goto customCompiler
if exist "C:\work\msvc\setup_x64.bat" (
    set "LOGL_MSVC_SETUP=C:\work\msvc\setup_x64.bat"
    goto customCompiler
)
goto visualStudio

:customCompiler
if not exist "%LOGL_MSVC_SETUP%" (
    echo Compiler setup script not found: "%LOGL_MSVC_SETUP%"
    exit /b 1
)
cmd /c exit 0
call "%LOGL_MSVC_SETUP%"
if errorlevel 1 exit /b 1
where cl >nul 2>nul
if errorlevel 1 goto missingCompiler
goto dependencies

:visualStudio
set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" goto missingCompiler
set "vsDir="
for /f "usebackq delims=" %%I in (`"%vswhere%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "vsDir=%%I"
if not defined vsDir goto missingCompiler
call "%vsDir%\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 exit /b 1

:dependencies
if /I "%configuration%"=="clangd" goto clangd
if not exist "%rootDir%\ext\sdl3\lib\SDL3.lib" goto missingDependencies
if not exist "%rootDir%\bin\SDL3.dll" goto missingDependencies
if not exist "%rootDir%\bin\opengl32.dll" goto missingDependencies
if not exist "%rootDir%\code\clangd.py" goto compile
python --version >nul 2>nul
if errorlevel 1 goto compile
python "%rootDir%\code\clangd.py"
if errorlevel 1 echo Could not refresh clangd commands. Run code\build.bat clangd to retry.

:compile
if not exist "%rootDir%\build" mkdir "%rootDir%\build"
pushd "%rootDir%\build"
if errorlevel 1 exit /b 1
cl /nologo /fp:fast /GR- /EHa- /Oi /WX /W4 /wd4201 /wd4100 /wd4189 /wd4505 /DLOGL_ROOT=1 /DLOGL_WIN32=1 /D_CRT_SECURE_NO_WARNINGS /FC %modeFlags% "%rootDir%\code\logl_main.cpp" /Fe:LOGL.exe /I "%rootDir%\ext\sdl3\include" /I "%rootDir%\ext\glad\include" /I "%rootDir%\ext\glad\src" /I "%rootDir%\ext\stb_image\include" /I "%rootDir%\ext\glm\include" /I "%rootDir%\ext\cgltf\include" /I "%rootDir%\ext\imgui\include" /I "%rootDir%\ext\imgui\include\backends" /link /LIBPATH:"%rootDir%\ext\sdl3\lib" /incremental:no /opt:ref /delayload:SDL3.dll delayimp.lib user32.lib gdi32.lib winmm.lib SDL3.lib opengl32.lib
set "buildResult=%errorlevel%"
popd
exit /b %buildResult%

:missingCompiler
echo MSVC x64 tools not found. Set LOGL_MSVC_SETUP to your setup batch file or use an x64 Native Tools Command Prompt.
exit /b 1
:missingDependencies
echo Required dependency files not found. See README.md for the required layout.
exit /b 1

:clangd
python "%rootDir%\code\clangd.py"
exit /b %errorlevel%
