@echo off

call common.cmd
set CFG_DEVELOP=0

if "%TARGET%"=="" set TARGET=%CFG_PROJECT%
if "%AUTOBUILD%"=="" set AUTOBUILD=0

if defined CFG_WIN32_NMAKE (
    set BUILD_CMD=post_build.cmd
    set RUN_CMD=%CMAKE_SOURCE_DIR%\build\%CFG_PLATFORM%\%CFG_PROJECT%\project\%TARGET%\exec.cmd

    mkdir %CFG_PROJECT%

    if not exist "%CMAKE_SOURCE_DIR%/project/%CFG_PROJECT%/config.cmake" (
        if "%AUTOBUILD%"=="1" (
            mconf --autowrite --prefix "CFG_" --cmakefile %CFG_PROJECT%/config.cmake --cfgfile "%CFG_PROJECT%/.config %CMAKE_SOURCE_DIR%/project/%CFG_PROJECT%/Kconfig.win32"
        ) else (
            qconf --prefix "CFG_" --cmakefile %CFG_PROJECT%/config.cmake --cfgfile %CFG_PROJECT%/.config "%CMAKE_SOURCE_DIR%/project/%CFG_PROJECT%/Kconfig.win32"
        )
        if errorlevel 1 exit /b
    ) else (
        call post_build.cmd
    )
) else (
    if not exist %CFG_PROJECT%/config.cmake (
        mkdir %CFG_PROJECT%
        title Configuration
        chcp 437 > nul

        if "%AUTOBUILD%"=="1" (
            mconf --autowrite --prefix "CFG_" --cmakefile %CFG_PROJECT%/config.cmake --cfgfile %CFG_PROJECT%/.config "%CMAKE_SOURCE_DIR%/project/%CFG_PROJECT%/Kconfig.win32"
        ) else (
            qconf --prefix "CFG_" --cmakefile %CFG_PROJECT%/config.cmake --cfgfile %CFG_PROJECT%/.config "%CMAKE_SOURCE_DIR%/project/%CFG_PROJECT%/Kconfig.win32"
        )
        if errorlevel 1 exit /b
    )
    cd %CFG_PROJECT%
    if "%VS12%"=="1" (
        cmake.exe -G"Visual Studio 12 2013" -T v120_xp "%CMAKE_SOURCE_DIR%"
    ) else (
        cmake.exe -G"Visual Studio 9 2008" "%CMAKE_SOURCE_DIR%"
    )
    if errorlevel 1 exit /b
)

if "%AUTOBUILD%"=="1" (
    MSBuild "%CMAKE_SOURCE_DIR%\build\%CFG_PLATFORM%\%CFG_PROJECT%\%CFG_PROJECT%.sln"
)
