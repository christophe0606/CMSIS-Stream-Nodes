# Recorder example

The recorder is a small CMSIS-Stream application that captures microphone audio and sends each block to a null sink. Its graph, target selection, and parameters are defined in [`create.py`](create.py).

Generate the graph from the repository root:

```powershell
uv run examples/recorder/create.py --runner zephyr --board fvp_cs300
```

The generated graph is written to `runner_common/app_graph/` and is then built with the selected runner.

See [Configuring and generating an application](../../documentation/configuring_application.md) for a step-by-step explanation of this example.
