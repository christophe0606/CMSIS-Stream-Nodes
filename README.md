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

## Configuration

```bash
python create.py --runner zephyr --board fvp_cs300
```


```powershell
cd examples/recorder/python
python create.py
```

Configure and build with the installed CMSIS-Stream POSIX runtime package:

```powershell
cmake -S examples/recorder -B examples/recorder/build -DCMSIS_STREAM_INSTALL_PREFIX=C:/cmake_packages
cmake --build examples/recorder/build
```

Run:

```powershell
.\examples\recorder\build\Debug\recorder.exe
```

The recorder stream runs until the POSIX runtime is stopped.
