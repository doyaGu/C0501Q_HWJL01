@echo off

set BOOTLOADER=1
rem set AUTOBUILD=1

setlocal EnableDelayedExpansion
set TARGETCMD=%~n0.cmd
call %TARGETCMD:_all=!!%

@if not defined NO_PAUSE pause
