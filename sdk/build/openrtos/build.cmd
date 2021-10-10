@echo off
rem set VERBOSE=1

call common.cmd
chcp 437 > nul

if not exist %CFG_PROJECT% mkdir %CFG_PROJECT%

if "%BOOTLOADER%"=="" set BOOTLOADER=0
if "%CODEC%"=="" set CODEC=0
if "%CODEC_EX%"=="" set CODEC_EX=0
if "%CODEC_IT9910%"=="" set CODEC_IT9910=0
if "%CODEC_EX_IT9910%"=="" set CODEC_EX_IT9910=0
if "%TARGET%"=="" set TARGET=%CFG_PROJECT%
if "%AUTOBUILD%"=="" set AUTOBUILD=0
if "%MAKEJOBS%"=="" set MAKEJOBS=4
set BUILD_CMD=post_build.cmd
set RUN_CMD=%CMAKE_SOURCE_DIR%\build\%CFG_PLATFORM%\%CFG_PROJECT%\project\%TARGET%\exec.cmd
rem # for distcc+
rem set DISTCC_VERBOSE=1
set DISTCC_DIR=%CMAKE_SOURCE_DIR%\build\%CFG_PLATFORM%\%CFG_PROJECT%\
set TMPDIR=%CMAKE_SOURCE_DIR%\build\%CFG_PLATFORM%\%CFG_PROJECT%\
rem # for distcc-
set NO_PAUSE=1
set CFG_DEVELOP=0

if "%BOOTLOADER%"=="1" (
    set PRESETTINGS=--loadcfg %CMAKE_SOURCE_DIR%/build/_presettings/_config_bootloader
) else (
    set PRESETTINGS=
)

if not "%PRESETTING%"=="" (
    if not "%PRESETTINGS%"=="" (
        set PRESETTINGS=%PRESETTINGS% --loadcfg %CMAKE_SOURCE_DIR%/build/_presettings/%PRESETTING%
    ) else (
        set PRESETTINGS=--loadcfg %CMAKE_SOURCE_DIR%/build/_presettings/%PRESETTING%
    )
)

if not exist "%CMAKE_SOURCE_DIR%/project/%CFG_PROJECT%/config.cmake" (
    if "%AUTOBUILD%"=="1" (
        mconf --autowrite --prefix "CFG_" --cmakefile %CFG_PROJECT%/config.cmake --cfgfile %CFG_PROJECT%/.config %PRESETTINGS% "%CMAKE_SOURCE_DIR%/project/%CFG_PROJECT%/Kconfig"
    ) else (
        qconf --fontsize 11 --prefix "CFG_" --cmakefile %CFG_PROJECT%/config.cmake --cfgfile %CFG_PROJECT%/.config %PRESETTINGS% "%CMAKE_SOURCE_DIR%/project/%CFG_PROJECT%/Kconfig"
    )
    if errorlevel 1 exit /b
    if "%AUTOBUILD%"=="1" (
        call post_build.cmd
    )
) else (
    call post_build.cmd
)
