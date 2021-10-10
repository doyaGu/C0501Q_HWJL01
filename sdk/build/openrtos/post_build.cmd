@echo off

chcp 437 > nul

set NINJA=0
set CYGWIN=nodosfilewarning
if exist %CFG_PROJECT%/project/%TARGET%/%TARGET% del %CFG_PROJECT%\project\%TARGET%\%TARGET%

set PROJECT_NAME=%CFG_PROJECT%

if "%CODEC%" == "1" (
    if "%MAKECLEAN%"=="1" (
        if exist risc1_code\CMakeFiles (
            del /f/q/s risc1_code\CMakeFiles
            rd /q/s risc1_code\CMakeFiles
        )
        if exist risc1_code\CMakeCache.txt (
            del /f/q/s risc1_code\CMakeCache.txt
        )

        if exist codec\CMakeFiles (
            del /f/q/s codec\CMakeFiles
            rd /q/s codec\CMakeFiles
        )
        if exist codec\CMakeCache.txt (
            del /f/q/s codec\CMakeCache.txt
        )
    )
    call :buildProject risc1_code
    call :buildProject codec
)

if "%CODEC_EX%" == "1" (
    if "%MAKECLEAN%"=="1" (
        if exist codec_ex\CMakeFiles (
            del /f/q/s codec_ex\CMakeFiles
            rd /q/s codec_ex\CMakeFiles
        )
        if exist codec_ex\CMakeCache.txt (
            del /f/q/s codec_ex\CMakeCache.txt
        )
    )
    call :buildProject codec_ex
)

if "%CODEC_IT9910%" == "1" (
    if "%MAKECLEAN%"=="1" (
        if exist codec_it9910\CMakeFiles (
            del /f/q/s codec_it9910\CMakeFiles
            rd /q/s codec_it9910\CMakeFiles
        )
        if exist codec_it9910\CMakeCache.txt (
            del /f/q/s codec_it9910\CMakeCache.txt
        )
    )
    call :buildProject codec_it9910
)

if "%CODEC_EX_IT9910%" == "1" (
    if "%MAKECLEAN%"=="1" (
        if exist codec_ex_it9910\CMakeFiles (
            del /f/q/s codec_ex_it9910\CMakeFiles
            rd /q/s codec_ex_it9910\CMakeFiles
        )
        if exist codec_ex_it9910\CMakeCache.txt (
            del /f/q/s codec_ex_it9910\CMakeCache.txt
        )
    )
    call :buildProject codec_ex_it9910
)

if "%CODEC_IT9850%" == "1" (
    if "%MAKECLEAN%"=="1" (
        if exist codec_it9850\CMakeFiles (
            del /f/q/s codec_it9850\CMakeFiles
            rd /q/s codec_it9850\CMakeFiles
        )
        if exist codec_it9850\CMakeCache.txt (
            del /f/q/s codec_it9850\CMakeCache.txt
        )
    )
    call :buildProject codec_it9850
)

if "%CODEC_EX_IT9850%" == "1" (
    if "%MAKECLEAN%"=="1" (
        if exist codec_ex_it9850\CMakeFiles (
            del /f/q/s codec_ex_it9850\CMakeFiles
            rd /q/s codec_ex_it9850\CMakeFiles
        )
        if exist codec_ex_it9850\CMakeCache.txt (
            del /f/q/s codec_ex_it9850\CMakeCache.txt
        )
    )
    call :buildProject codec_ex_it9850
)

if "%ALT_CPU_IT9850%" == "1" (
    if "%MAKECLEAN%"=="1" (
        if exist alt_cpu_it9850\CMakeFiles (
            del /f/q/s alt_cpu_it9850\CMakeFiles
            rd /q/s alt_cpu_it9850\CMakeFiles
        )
        if exist alt_cpu_it9850\CMakeCache.txt (
            del /f/q/s alt_cpu_it9850\CMakeCache.txt
        )
    )
    call :buildProject alt_cpu_it9850
)

if "%RISC_TEST%" == "1" (
    call :buildProject risc1_code
    call :buildProject risc2_code
    call :buildProject risc1_code_it9850
    call :buildProject risc2_code_it9850
)

if "%MAKECLEAN%"=="1" (
    if exist %PROJECT_NAME%\CMakeFiles (
        del /f/q/s %PROJECT_NAME%\CMakeFiles
        rd /q/s %PROJECT_NAME%\CMakeFiles
    )
    if exist %PROJECT_NAME%\CMakeCache.txt (
        del /f/q/s %PROJECT_NAME%\CMakeCache.txt
    )
)
call :buildProject %PROJECT_NAME%
goto :eof

rem ###########################################################################
rem The function to build project
rem ###########################################################################
:buildProject
SETLOCAL
set LOCAL_PROJECT_NAME=%1
set CFG_PROJECT=%LOCAL_PROJECT_NAME%
if not exist %LOCAL_PROJECT_NAME% mkdir %LOCAL_PROJECT_NAME%
pushd %LOCAL_PROJECT_NAME%

if "%NINJA%" == "0" (
    if exist CMakeCache.txt (
        cmake.exe -G"Unix Makefiles" "%CMAKE_SOURCE_DIR%"
    ) else (
        cmake.exe -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="%CFG_TOOLCHAIN_FILE%" "%CMAKE_SOURCE_DIR%"
    )
    if errorlevel 1 exit /b

    if "%MAKECLEAN%"=="1" (
        echo "Clean build..."
        make clean
    )

    if "%MAKEJOBS%"=="" (
        make -j 1 VERBOSE=%VERBOSE%
    ) else (
        make -j %MAKEJOBS% VERBOSE=%VERBOSE%
    )
) else (
    if "%VERBOSE%" == "1" (
        set VERBOSEOUT="-v"
    ) else (
        set VERBOSEOUT=""
    )

    if exist CMakeCache.txt (
        cmake.exe -G Ninja "%CMAKE_SOURCE_DIR%"
    ) else (
        cmake.exe -G Ninja -DCMAKE_TOOLCHAIN_FILE="%CFG_TOOLCHAIN_FILE%" "%CMAKE_SOURCE_DIR%"
    )
    if errorlevel 1 exit /b

    if "%MAKECLEAN%"=="1" (
        echo "Clean build..."
        ninja -t clean
    )

    if "%MAKEJOBS%"=="" (
        ninja -j 1 %VERBOSEOUT%
    ) else (
        ninja -j %MAKEJOBS% %VERBOSEOUT%
    )
)
popd

ENDLOCAL
