@echo off

rem set AUTOBUILD=1
set CFG_PROJECT=%~n0
rem set CFG_WIN32_NMAKE=1
call build.cmd

@if not defined NO_PAUSE pause
