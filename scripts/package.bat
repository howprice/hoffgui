
@echo off

if "%VisualStudioVersion%"=="" (
	echo Please run in Visual Studio Native Tools Command Prompt or run vcvarsall.bat to ensure cpack is on the path
	EXIT /B 1
)

REM Set the current directory to the location of the batch script, using the %0 parameter
REM This allows the script to be called from anywhere
cd "%~dp0"

REM Run cpack in the build folder
REM Add --verbose to see more output
pushd ..\build
cpack -C Release || EXIT /B 1
popd
EXIT /B
