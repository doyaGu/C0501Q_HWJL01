@echo off

chcp 437 > nul

cd %CFG_PROJECT%
if exist project/%TARGET%/%TARGET% del project\%TARGET%\%TARGET%

cmake.exe -G"NMake Makefiles" "%CMAKE_SOURCE_DIR%"
if errorlevel 1 exit /b

nmake.exe
