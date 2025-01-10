@echo off

set CompilerFlags=-I./ -nologo -O2 -arch:AVX2 -fp:fast -fp:except- -GR- -EHa- -Zo -W4 -GS- -Z7 -wd4100 -wd4127 -wd4189 -wd4201
set LinkerFlags=-incremental:no -opt:ref user32.lib kernel32.lib Advapi32.lib

clang-cl %CompilerFlags% NKTests/Tests.cpp /link /NODEFAULTLIB -OUT:Tests.exe /SUBSYSTEM:console %LinkerFlags%
clang-cl %CompilerFlags% NKTests/WindowTest.cpp /link /NODEFAULTLIB -OUT:WindowTest.exe /SUBSYSTEM:console %LinkerFlags% ThirdPartyLibs.lib OpenGL32.lib Gdi32.lib
