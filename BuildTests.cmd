@echo off

set WARNINGS1=-Wall -Wextra -Wconversion -Wno-sign-conversion -Wno-unused-but-set-variable -Wno-unused-parameter
set WARNINGS2=-Wno-unused-variable -Wno-unused-function -Wno-char-subscripts -Wno-nullability-completeness
set WARNINGS3=-Wno-missing-field-initializers -Wno-missing-braces
set WARNINGS=%WARNINGS1% %WARNINGS2% %WARNINGS3%

set CFLAGS=%WARNINGS% -nostdlib -nostdlib++ -fuse-ld=lld -Wl,-subsystem:console

set DEBUG_FLAGS=-DBUILD_DEBUG -g3
set RELEASE_FLAGS=-O3 -march=native -mtune=native -fomit-frame-pointer

set BUILD_MODE=%DEBUG_FLAGS%

set INCLUDES=-I./

clang++ %BUILD_MODE% %CFLAGS% %INCLUDES% NKTests/Tests.cpp -o Tests.exe
