@echo off

REM Set the current directory to the location of the batch script, using the %0 parameter
REM This allows the script to be called from anywhere
cd "%~dp0"

pushd ..
if exist build (
    rmdir build /s /q
    echo Deleted build folder
) else (
    echo build folder does not exist
)
popd
