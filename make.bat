@echo off
if "%_VCVARS%" == "" (
	set "_VCVARS=1"
	vcvarsall x64
)
if "%1" == "" (
	cl /nologo /W4 /wd4706 /wd4996 /wd4100 /Zi /Od /DEBUG /DDEBUG=1 main.c SDL2\lib\x64\SDL2.lib SDL2\lib\x64\SDL2main.lib /Fe:sandbox shell32.lib
)
if "%1" == "release" (
	cl /nologo /W4 /wd4706 /wd4996 /wd4100 /O2 main.c SDL2\lib\x64\SDL2.lib SDL2\lib\x64\SDL2main.lib /Fe:sandbox shell32.lib
)
