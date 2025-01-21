@echo off

set RELEASE_BUILD=-O2
set DEBUG_BUILD=-Z7
set BUILD_MODE=%DEBUG_BUILD%

set CompilerFlags=-I./ -nologo -arch:AVX2 -fp:fast -fp:except- -GR- -EHa- -Zo -W4 -GS- -wd4100 -wd4127 -wd4189 -wd4201 %BUILD_MODE%
set LinkerFlags=-incremental:no -opt:ref user32.lib kernel32.lib Advapi32.lib

rem clang-cl %CompilerFlags% NKTests/Tests.cpp /link -OUT:Tests.exe /NODEFAULTLIB /SUBSYSTEM:console %LinkerFlags%
rem clang-cl %CompilerFlags% NKTests/WindowTest.cpp /link -OUT:WindowTest.exe /NODEFAULTLIB /SUBSYSTEM:console %LinkerFlags% Gdi32.lib d3d11.lib dwrite.lib D3DCompiler.lib dxguid.lib d2d1.lib
clang-cl %CompilerFlags% Editor/Editor.cpp /link -OUT:Editor.exe /NODEFAULTLIB /SUBSYSTEM:console %LinkerFlags% Gdi32.lib d3d11.lib dwrite.lib D3DCompiler.lib dxguid.lib d2d1.lib
