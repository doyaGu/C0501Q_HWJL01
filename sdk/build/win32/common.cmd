@if defined COMMON_DEFINED goto end
set COMMON_DEFINED=1

call :get_cfg_platform "%cd%"

rem ==========================================================================
rem set CMAKE_SOURCE_DIR=build\win32\..\..\
rem ==========================================================================
pushd ..\..
for /f "delims=" %%a in ('cd') do set CMAKE_SOURCE_DIR=%%a
popd

set CFG_BUILDPLATFORM=%CFG_PLATFORM%
set CMAKE_ROOT=%CMAKE_SOURCE_DIR%\tool
set PATH=%CMAKE_ROOT%\bin;%PATH%

if "%AUTOBUILD%"=="1" (
    if exist "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 12.0/VC/include
        set CFG_VC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 12.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files ^(x86^)/Microsoft SDKs/Windows/v7.1A/Include
        set VS=3
        set VS12=1
    )

    if exist "C:/Program Files/Microsoft Visual Studio 12.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files/Microsoft Visual Studio 12.0/VC/include
        set CFG_VC_PATH=C:/Program Files/Microsoft Visual Studio 12.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files/Microsoft SDKs/Windows/v7.1A/Include
        set VS=2
        set VS12=1
    )

    if exist "C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 9.0/VC/include
        set CFG_VC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 9.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files ^(x86^)/Microsoft SDKs/Windows/v6.0A/Include
        set VS=0
        set VS12=
    )

    if exist "C:/Program Files/Microsoft Visual Studio 9.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files/Microsoft Visual Studio 9.0/VC/include
        set CFG_VC_PATH=C:/Program Files/Microsoft Visual Studio 9.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files/Microsoft SDKs/Windows/v6.0A/Include
        set VS=1
        set VS12=
    )

    if exist "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 14.0/VC/include
        set CFG_VC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 14.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files ^(x86^)/Microsoft SDKs/Windows/v7.1A/Include
        set VS=5
        set VS14=1
    )

    if exist "C:/Program Files/Microsoft Visual Studio 14.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files/Microsoft Visual Studio 14.0/VC/include
        set CFG_VC_PATH=C:/Program Files/Microsoft Visual Studio 14.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files/Microsoft SDKs/Windows/v7.1A/Include
        set VS=4
        set VS14=1
    )
) else (
    if exist "C:/Program Files (x86)/Microsoft Visual Studio 9.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 9.0/VC/include
        set CFG_VC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 9.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files ^(x86^)/Microsoft SDKs/Windows/v6.0A/Include
        set VS=0
        set VS12=
    )

    if exist "C:/Program Files/Microsoft Visual Studio 9.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files/Microsoft Visual Studio 9.0/VC/include
        set CFG_VC_PATH=C:/Program Files/Microsoft Visual Studio 9.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files/Microsoft SDKs/Windows/v6.0A/Include
        set VS=1
        set VS12=
    )

    if exist "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 12.0/VC/include
        set CFG_VC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 12.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files ^(x86^)/Microsoft SDKs/Windows/v7.1A/Include
        set VS=3
        set VS12=1
    )

    if exist "C:/Program Files/Microsoft Visual Studio 12.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files/Microsoft Visual Studio 12.0/VC/include
        set CFG_VC_PATH=C:/Program Files/Microsoft Visual Studio 12.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files/Microsoft SDKs/Windows/v7.1A/Include
        set VS=2
        set VS12=1
    )

     if exist "C:/Program Files (x86)/Microsoft Visual Studio 14.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 14.0/VC/include
        set CFG_VC_PATH=C:/Program Files ^(x86^)/Microsoft Visual Studio 14.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files ^(x86^)/Microsoft SDKs/Windows/v7.1A/Include
        set VS=5
        set VS14=1
    )

    if exist "C:/Program Files/Microsoft Visual Studio 14.0/VC" (
        set CFG_VC_INC_PATH=C:/Program Files/Microsoft Visual Studio 14.0/VC/include
        set CFG_VC_PATH=C:/Program Files/Microsoft Visual Studio 14.0/VC
        set CFG_WINSDK_INC_PATH=C:/Program Files/Microsoft SDKs/Windows/v7.1A/Include
        set VS=4
        set VS14=1
    )
)

set VS=%VS%

call "%CFG_VC_PATH%\vcvarsall.bat"

:end
goto :eof

:get_cfg_platform
rem ==========================================================================
rem set CFG_PLATFORM=win32 from the current directory name build\[win32]
rem ==========================================================================
set CFG_PLATFORM=%~n1
goto :eof
