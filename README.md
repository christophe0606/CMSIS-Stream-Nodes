# CMSIS-Stream Nodes

Build AI and multimedia processing graphs once and run them across embedded targets and desktop operating systems. This project is based on [CMSIS-Stream](https://github.com/ARM-software/CMSIS-Stream), which describes and generates the scheduled dataflow graph.

The project adds:

* Reusable CMSIS-Stream nodes for audio, AI, and multimedia processing
* A Python-based way to configure the parameters of every node in an application
* Target-specific implementations and hardware initialization for multiple runners and boards
* Runner projects for CMSIS-RTOS, Zephyr, Linux, macOS, and Windows

Read [How the project works](documentation/principles.md) for the complete graph-description, code-generation, parameter, hardware, runtime-thread, event, and application-integration model.

## Runners

Use the runner documentation to configure and build the generated application for a target environment:

* [POSIX runner](documentation/posix_runner.md)
* [Zephyr runner](documentation/zephyr_runner.md)
* [CMSIS runner](documentation/cmsis_runner.md)

## Configuration

When you generate code for a graph, you need to select the runner and the board.

```bash
python examples/recorder/create.py --runner zephyr --board fvp_cs300
```

The CMSIS-Stream scheduler will be generated in `runner_common/app_graph`. You'll then have to build the right runner. Even if the scheduler is generated in common, it cannot be built for all runners if it uses nodes that are runner- or board-specific.

This script is available in each example folder. For instance: `examples/recorder/create.py`.

See [Configuring and generating an application](documentation/configuring_application.md) for a recorder-based walkthrough. The [project principles](documentation/principles.md#generating-and-building-an-application) explain how generation selects node implementations and board support.

## Network

The demo using a TFLite network currently uses only one network.
The format of this network depends on the target : posix or Ethos.
The network is selected in `app.py` when running the python script `create.py` for a given application.
