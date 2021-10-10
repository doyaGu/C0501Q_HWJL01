@echo off

set CFG_PROJECT=%~n0
set CODEC=1
set CODEC_EX=1
rem set AUTOBUILD=1

call build.cmd

@if not defined NO_PAUSE pause
