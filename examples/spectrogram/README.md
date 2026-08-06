# Spectrogram example

This example captures microphone audio, computes a spectrogram, and sends framed spectrogram data over the target's serial output.

Generate the graph from the repository root. For example, for the Alif Ensemble E7 board with the CMSIS runner:

```powershell
uv run examples/spectrogram/create.py --runner cmsis --board AlifE7
```

After building and starting the selected runner, use the terminal display tool to connect to its serial port:

```powershell
uv run examples/tools/spectrogram_terminal.py COM4
```

Omit the port name to select from the detected serial ports. The default baud rate is 115200; use `--baudrate` to override it and press `q` to quit.
