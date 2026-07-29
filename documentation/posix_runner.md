# POSIX runner

## Visual Studio Code configuration

Add this to `.vscode/settings.json` if you want to use the Visual Studio Code CMake extension with presets:

```json
"cmake.sourceDirectory": "${workspaceFolder}/posix_runner"
```

This setting conflicts with the Zephyr build configuration.

## Prerequisites

Configure and build with the installed CMSIS-Stream POSIX runtime package. To learn how to install the CMake package, see the POSIX runner README in the [CMSIS-Stream repository](https://github.com/ARM-software/CMSIS-Stream).

Configure and install the [CMSIS-DSP](https://github.com/ARM-software/CMSIS-DSP) CMake package. See the CMSIS-DSP README for build and installation instructions.

If the graph uses POSIX audio capture nodes such as `MicrophoneSource`, the PortAudio CMake package must also be installed and discoverable by CMake. Without PortAudio, POSIX graphs that do not use audio capture can still build, but microphone capture will not be available.

## Build and run

The following commands assume that the packages have been installed in `C:/cmake_packages` on Windows:

```powershell
cmake -S posix_runner -B posix_runner/build -DCMSIS_STREAM_INSTALL_PREFIX=C:/cmake_packages
cmake --build posix_runner/build
```

Run the application:

```powershell
posix_runner\build\Release\app.exe
```

The recorder stream runs for ten audio blocks.

[Back to the project README](../README.md)
