@echo off

set rootDir=%cd%

set compilerFlags= -MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -DLOGL_INTERNAL=1 -DLOGL_SLOW=1 -DLOGL_ROOT=1 -DLOGL_WIN32=1 -FC -Z7

set includeDirs= /I %rootDir%\ext\sdl3\include /I %rootDir%\ext\glad\include /I %rootDir%\ext\glad\src\ /I %rootDir%\ext\stb_image\include /I %rootDir%\ext\glm\include

set linkDirs= /LIBPATH:%rootDir%/ext/sdl3/lib/ /LIBPATH:%rootDir%/ext/glad/src

set linkerFlags= user32.lib gdi32.lib winmm.lib SDL3.lib opengl32.lib

IF NOT EXIST .\build mkdir .\build
pushd .\build

cl %compilerFlags% ..\code\sdl_main.cpp %includeDirs% /link %linkDirs% %linkerFlags%

popd
