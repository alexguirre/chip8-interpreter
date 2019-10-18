# Chip-8 Interpreter

[![CI](https://github.com/alexguirre/chip8-interpreter/workflows/CI/badge.svg)](https://github.com/alexguirre/chip8-interpreter/actions?workflow=CI)

A Chip-8 interpreter written in C++17.

![Interpreter demo](https://i.imgur.com/jPXOMiE.gif)

## Dependencies

- [dear imgui](https://github.com/ocornut/imgui)
- [doctest](https://github.com/onqtam/doctest)
- [gl3w](https://github.com/skaslev/gl3w)
- [ms-gsl](https://github.com/Microsoft/GSL)
- [SDL2](https://www.libsdl.org/download-2.0.php)
- [TCLAP](http://tclap.sourceforge.net/)

## Quick Start

Prerequisites:

- Windows 10 or Linux
- Visual Studio 2019 (on Windows)
- Git
- CMake 3.12.4 or newer

On Windows, enter the [Visual Studio development environment](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2019) with x64 native tools to get access to the CMake version included with Visual Studio.

1. Install [vcpkg](https://github.com/Microsoft/vcpkg):

    ```console
    > git clone https://github.com/microsoft/vcpkg.git
    > cd vcpkg
    PS> .\bootstrap-vcpkg.bat
    $ ./bootstrap-vcpkg.sh
    ```

1. Install the dependencies:

    ```console
    PS> .\vcpkg --triplet x64-windows-static install imgui doctest gl3w ms-gsl sdl2 tclap
    $ ./vcpkg --triplet x64-linux install imgui doctest gl3w ms-gsl sdl2 tclap
    > cd ..
    ```

1. Run CMake:

    ```console
    PS> mkdir src\build
    PS> cd src\build
    PS> cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE="..\..\vcpkg\scripts\buildsystems\vcpkg.cmake" ..
    $ mkdir src/build
    $ cd src/build
    $ cmake -DVCPKG_TARGET_TRIPLET=x64-linux -DCMAKE_TOOLCHAIN_FILE="../../vcpkg/scripts/buildsystems/vcpkg.cmake" ..
    ```

1. Build:

    ```console
    > cmake --build . --config Release
    ```

1. Compiled binaries will be in `src/build/bin`.

## Usage

Run `c8` with the argument `--help` to see usage help.

```console
> ./c8 --help
```

## References

- http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
- https://github.com/dmatlack/chip8
- https://github.com/btimofeev/emuchip
- https://github.com/mattmikolay/chip-8
