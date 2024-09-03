@echo off

REM Save the current directory
set "originalDir=%CD%"

REM Set the current directory to the location of the batch script, using the %0 parameter
REM This allows the script to be called from anywhere
cd "%~dp0"

if exist ..\data\imgui.ini (
    del ..\data\imgui.ini
    echo Deleted data\imgui.ini
) else (
    echo data\imgui.ini does not exist
)

if exist "%APPDATA%\TTE\hoffgui\hoffgui.ini" (
    del "%APPDATA%\TTE\hoffgui\hoffgui.ini"
    echo Deleted %APPDATA%\TTE\hoffgui\hoffgui.ini
) else (
    echo %APPDATA%\TTE\hoffgui\hoffgui.ini does not exist
)

REM Return to the original directory
cd "%originalDir%"
