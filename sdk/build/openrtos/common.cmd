@echo off
set CYGWIN=nodosfilewarning
@if defined COMMON_DEFINED goto end
set COMMON_DEFINED=1
set GCC_COLORS=1

call :get_cfg_platform "%cd%"

pushd ..\..
for /f "delims=" %%a in ('cd') do set CMAKE_SOURCE_DIR=%%a
popd

set CFG_BUILDPLATFORM=%CFG_PLATFORM%
set CMAKE_ROOT=%CMAKE_SOURCE_DIR%\tool
set CFG_TOOLCHAIN_FILE=%CMAKE_SOURCE_DIR%\%CFG_PLATFORM%\toolchain.cmake

for /f "tokens=3 delims=() " %%a in ('find "CMAKE_FIND_ROOT_PATH " "%CFG_TOOLCHAIN_FILE%"') do set TOOLCHAIN=%%a

set TOOLCHAIN=%TOOLCHAIN:/=\%
set PATH=%CMAKE_ROOT%\bin;%TOOLCHAIN%\bin;C:\ITEGCC\bin;%PATH%
title %CFG_PLATFORM%-%CFG_PROJECT%

:end
goto :eof

:get_cfg_platform
set CFG_PLATFORM=%~n1

goto :eof
