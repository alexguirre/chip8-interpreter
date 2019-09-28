# Chip-8 Interpreter

A Chip-8 interpreter written in C++17.

## Dependencies

- [dear imgui](https://github.com/dmatlack/chip8)
- [gl3w](https://github.com/skaslev/gl3w)
- [ms-gsl](https://github.com/Microsoft/GSL)
- [SDL2](https://www.libsdl.org/download-2.0.php)
- [TCLAP](http://tclap.sourceforge.net/)

## Building

### Windows / Visual Studio / vcpkg

1. Install [vcpkg](https://github.com/Microsoft/vcpkg):

```powershell
PS> git clone https://github.com/microsoft/vcpkg.git
PS> cd vcpkg
PS> .\bootstrap-vcpkg.bat
```

1. Hook up user-wide integration:

```powershell
PS> .\vcpkg integrate install
```

1. Install the dependencies:

```powershell
PS> .\vcpkg install imgui:x64-windows-static
PS> .\vcpkg install gl3w:x64-windows-static
PS> .\vcpkg install ms-gsl:x64-windows-static
PS> .\vcpkg install sdl2:x64-windows-static
PS> .\vcpkg install tclap:x64-windows-static
```

1. Open [chip8-interpreter.sln](src/chip8-interpreter.sln) in Visual Studio and build the project.

### Linux

TBD

## Usage

```console
.\c8-interpreter path\to\rom
```

## References

- http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
- https://github.com/dmatlack/chip8
- https://github.com/btimofeev/emuchip
