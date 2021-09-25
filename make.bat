@echo off
if "%_VCVARS%" == "" (
	set "_VCVARS=1"
	set "PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\bin\Hostx64\x64\;C:\Program Files (x86)\Windows Kits\10\bin\10.0.18362.0\x64\;%PATH%"
	set "INCLUDE=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\ATLMFC\include;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\include;C:\Program Files (x86)\Windows Kits\10\include\10.0.18362.0\ucrt;C:\Program Files (x86)\Windows Kits\10\include\10.0.18362.0\shared;C:\Program Files (x86)\Windows Kits\10\include\10.0.18362.0\um;C:\Program Files (x86)\Windows Kits\10\include\10.0.18362.0\winrt;C:\Program Files (x86)\Windows Kits\10\include\10.0.18362.0\cppwinrt;%INCLUDE%"
	set "LIB=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\ATLMFC\lib\x64;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\lib\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.18362.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.18362.0\um\x64;%LIB%"
	set "LIBPATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\ATLMFC\lib\x64;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\lib\x64;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\lib\x86\store\references;C:\Program Files (x86)\Windows Kits\10\UnionMetadata\10.0.18362.0;C:\Program Files (x86)\Windows Kits\10\References\10.0.18362.0;C:\Windows\Microsoft.NET\Framework64\v4.0.30319;%LIBPATH%"
)
if "%1" == "" (
	cl /nologo /W4 /wd4706 /wd4996 /wd4100 /Zi /Od /DEBUG /DDEBUG=1 main.c SDL2\lib\x64\SDL2.lib SDL2\lib\x64\SDL2main.lib /Fe:sandbox shell32.lib
)
if "%1" == "release" (
	cl /nologo /W4 /wd4706 /wd4996 /wd4100 /O2 main.c SDL2\lib\x64\SDL2.lib SDL2\lib\x64\SDL2main.lib /Fe:sandbox shell32.lib
)
