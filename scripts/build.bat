@echo off

if "%VisualStudioVersion%"=="" (
	echo Please run in Visual Studio Native Tools Command Prompt or run vcvarsall.bat to ensure smake is on the path
	EXIT /B 1
)

REM Set the current directory to the location of the batch script, using the %0 parameter
REM This allows the script to be called from anywhere
cd "%~dp0"

echo:
echo Building Debug
cmake --build ..\build --config Debug || EXIT /B 1

echo:
echo Building Release
cmake --build ..\build --config Release || EXIT /B 1

EXIT /B
