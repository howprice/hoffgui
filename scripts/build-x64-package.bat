
@echo off

REM Set the current directory to the location of the batch script, using the %0 parameter
REM This allows the script to be called from anywhere
cd "%~dp0"

REM n.b. CMake cannot build both 32 and 64 bit in the same build directory
set BUILD_DIR=build

pushd ..

echo Configuring
REM TODO: Why does vscode cmake use -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE ?
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -S. -B%BUILD_DIR% -G "Visual Studio 17 2022" -T host=x64 -A x64 || EXIT /B 1

echo Building
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build %BUILD_DIR% --config Release || EXIT /B 1

echo Packing
REM Run cpack in the build folder
REM Add --verbose to see more output
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cpack.exe" --config %BUILD_DIR%\CPackConfig.cmake -C Release || EXIT /B 1

popd

EXIT /B
