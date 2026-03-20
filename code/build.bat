@echo off

if not defined VSINSTALLDIR (
    call "c:\program files\microsoft visual studio\2022\community\vc\auxiliary\build\vcvarsall.bat" x64
)

set rootDir=%cd%

set compilerFlags= -MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -DLOGL_INTERNAL=1 -DLOGL_SLOW=1 -DLOGL_ROOT=1 -DLOGL_WIN32=1 -D_CRT_SECURE_NO_WARNINGS -FC -Z7

set includeDirs= /I %rootDir%\ext\sdl3\include /I %rootDir%\ext\glad\include /I %rootDir%\ext\glad\src\ /I %rootDir%\ext\stb_image\include /I %rootDir%\ext\glm\include /I %rootDir%\ext\cgltf\include /I %rootDir%\ext\imgui\include /I %rootDir%\ext\imgui\include\backends

set linkDirs= /LIBPATH:%rootDir%/ext/sdl3/lib/ /LIBPATH:%rootDir%/ext/glad/src

set linkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib SDL3.lib opengl32.lib

IF NOT EXIST .\build mkdir .\build
pushd .\build

REM C:\Users\Marcm\Downloads\imgui-1.91.5\imgui-1.91.5\examples\example_sdl3_opengl3
REM if not exist ImGui.lib (
REM    cl  -FoImGui.obj %includeDirs% /link %linkDirs% %linkerFlags% /c
REM    lib ImGui.obj
REM )

cl %compilerFlags% ..\code\logl_main.cpp /Fe.\LOGL.exe %includeDirs% /link %linkDirs% %linkerFlags%

popd
