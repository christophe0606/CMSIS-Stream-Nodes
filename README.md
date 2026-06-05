# README

Collection of nodes to use for building [CMSIS Stream](https://github.com/ARM-software/CMSIS-Stream) graphs.

Those nodes may be:

* Audio sources (micro)
* Audio sinks (speaker)
* Neural network models (executorch ...)
* Processing (MFCC, Color conversion ...)

As far as possible, I'll try to create nodes that can work on all platforms:

* CMSIS-RTOS API
* Zephyr
* Linux
* Mac
* Windows (in some cases)

The project also provides runner to be able to run CMSIS stream graphs on the different platforms:

* posix_runner
* zephyr_runner
* cmsis_runner

## Configuration

When you generate code for a graph, you need to select the runner and the board.

```bash
python create.py --runner zephyr --board fvp_cs300
```

The CMSIS Stream scheduler will be generated in `runner_common/app_graph`. You'll then have to build the right runner. Even if the scheduler is generated in common it cannot be built for all runners if it uses nodes that are runner and board specific.

## posix

Configure and build with the installed CMSIS-Stream POSIX runtime package.

To know how to install the cmake package, look at CMSIS Stream posix runner README in CMSIS Stream repository.

Configure and install the [CMSIS-DSP](https://github.com/ARM-software/CMSIS-DSP) cmake package. Look at CMSIS-DSP README in CMSIS-DSP github repository to see how to build and install the cmake package.

If the graph uses POSIX audio capture nodes such as `MicrophoneSource`, the PortAudio cmake package must also be installed and discoverable by CMake. Without PortAudio, POSIX graphs that do not use audio capture can still build, but microphone capture will not be available.

Below, it is assumed that the packages have been installed in `C:/cmake_packages` on Windows. 

```powershell
cmake -S posix_runner -B posix_runner/build -DCMSIS_STREAM_INSTALL_PREFIX=C:/cmake_packages
cmake --build posix_runner/build
```

Run:

```powershell
posix_runner\build\Release\app.exe
```

The recorder stream runs until the POSIX runtime is stopped.

# zephyr

Add this to the projects in your `west.yml` file:



```yaml
- name: cmsisstream
  url: https://github.com/ARM-software/CMSIS-Stream
  revision: main
  path: modules/lib/cmsisstream
```

Then build using vscode and the CMSIS Toolbox extension.

# cmsis

The CMSIS pack for CMSIS Stream has not yet been upstreamed but you can use it from your `csolution` file with something like:

```yaml
- pack: ARM::CMSIS-STREAM
  path: ../../CMSIS-Stream
```

And use the path to the CMSIS-Stream github repo that you have cloned.

Then build using vscode and the CMSIS Toolbox extension.

For audio on FVP, the AVH_FVP pack is used. It is not yet
available on the repository. So you need to clone the github repo: https://github.com/ARM-software/AVH

and then you can use the pack


```yaml
- pack: ARM::AVH_FVP
  path: ../../AVH
```