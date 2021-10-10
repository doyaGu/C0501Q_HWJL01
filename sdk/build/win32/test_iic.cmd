@echo off

rem set AUTOBUILD=1
set CFG_PROJECT=%~n0

call build.cmd

@if not defined NO_PAUSE pause
