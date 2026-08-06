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
cmake --build posix_runner/build --config Release
```

If you need TensorFlow Lite for Microcontrollers, set the following additional options:

```powershell
cmake -S posix_runner -B posix_runner/build -DUSE_TFLITE=ON -DTFLITE_PATH=C:/path/to/tflite-micro -DCMSIS_STREAM_INSTALL_PREFIX=C:/cmake_packages
```
* `USE_TFLITE` enables the CMake build of TensorFlow Lite for Microcontrollers.
* `TFLITE_PATH` is the path to the root of your `tflite-micro` checkout.

Run the application:

```powershell
posix_runner\build\Release\app.exe
```

The recorder stream runs for ten audio blocks.

[Back to the project README](../README.md)
