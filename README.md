# hoffgui

# Building

## Prerequisites

CMake and a C++ toolchain are required to build. OpenGL 3 drivers are required to run.

### Linux dependencies

    sudo apt install -y build-essential curl zip unzip tar pkg-config autoconf libtool ninja-build python3 python3-pip libegl1-mesa-dev

    pip install jinja2

Optional:

    sudo apt install clang

#### Raspberry Pi only

    sudo apt install libwayland-dev libxkbcommon-dev
    export VCPKG_FORCE_SYSTEM_BINARIES=1

The export command can be put in `.bashrc` to persist this setting.

## Get the source

This project uses submodules. Either clone with `--recurse-submodules`

    git clone https://github.com/howprice/hoffgui.git --recurse-submodules

 or, after cloning (without submodules) run `git submodule update --init --recursive`

    git clone https://github.com/howprice/hoffgui.git
    cd hoffgui
    git submodule update --init --recursive

## Configure an out-of-source build setup

From the repo root directory:

    cmake -S . -B build

## Build

Build the default development release with debug information:

    cmake --build build

or the optimised release build:

    cmake --build build --config Release

## Windows 

Configure CMake from the Visual Studio Native tools command prompt (e.g. Start -> *x64 Native Tools Command Prompt for VS2022*) to use the Visual Studio's CMake and toolchain. After configuring CMake, the generated solution `build/hoffgui.sln` can be opened with Visual Studio.


# Installing

On Windows, unzip the archive and run the executable.

On Mac mount the DMG and do the silly drag and drop thing

# Usage

# Credits

- [Dear ImGui](https:://github.com/ocornut/imgui) by Omar Cornut and contributors
- [Portable File Dialogs](https://github.com/samhocevar/portable-file-dialogs) by Sam Hocevar
- [Simple DirectMedia Layer (SDL2)](https://www.libsdl.org/) by Sam Lantinga and contributors
- [imgui_markdown](https://github.com/juliettef/imgui_markdown) by Juliette Foucaut & Doug Binks
- [cleanCppProject](https://github.com/kracejic/cleanCppProject) by kracejic
- [TTE](https://www.twitch.tv/djh0ffman) for inspiration, help and support

# Contributing

See [CONTRIBUTING](docs/CONTRIBUTING.md) file.

# License

This project is licensed under the MIT License. See [LICENSE](LICENSE.txt) for details.
