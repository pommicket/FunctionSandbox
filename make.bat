@echo off
if "%_VCVARS%" == "" (
	set "_VCVARS=1"
	call vcvarsall x64
)
if "%1" == "" (
	cl /nologo /W4 /wd4706 /wd4996 /wd4100 /wd4204 /Zi /Od /DEBUG /DDEBUG=1 main.c /I SDL2\include SDL2\lib\x64\SDL2.lib SDL2\lib\x64\SDL2main.lib /Fe:sandbox shell32.lib
)
if "%1" == "release" (
	cl /nologo /W4 /wd4706 /wd4996 /wd4100 /wd4204 /O2 main.c /I SDL2\include SDL2\lib\x64\SDL2.lib SDL2\lib\x64\SDL2main.lib /Fe:sandbox shell32.lib
	rd /s/q FunctionSandbox
	mkdir FunctionSandbox
	mkdir FunctionSandbox\sandboxes
	copy sandbox.exe FunctionSandbox
	copy sandboxes\*.txt FunctionSandbox\sandboxes
	copy example.png FunctionSandbox
	copy SDL2.dll FunctionSandbox
	del sandbox-windows.zip
	7z a sandbox-windows.zip FunctionSandbox
	rd /s/q FunctionSandbox
)
