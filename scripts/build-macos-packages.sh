#!/bin/bash

# cd out of scripts/ into root
pushd .. > /dev/null

echo Configuring
cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/clang -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/clang++ -S . -B build -G "Unix Makefiles"

echo Building
cmake --build build --config Release --target all

echo Packaging
# Run cpack in the build folder
# Use --verbose for more info
cpack --config build/CPackConfig.cmake -C Release

popd > /dev/null
