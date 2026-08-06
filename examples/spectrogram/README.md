# Spectrogram example

This example captures microphone audio, computes a spectrogram, and sends framed spectrogram data over the target's serial output or the POSIX runner's standard output.

Generate the graph from the repository root. For example, for the Alif Ensemble E7 board with the CMSIS runner:

```powershell
uv run examples/spectrogram/create.py --runner cmsis --board AlifE7
```

After building and starting the selected runner, use the terminal display tool to connect to its serial port:

```powershell
uv run examples/tools/spectrogram_terminal.py COM4
```

Omit the port name to select from the detected serial ports. The default baud rate is 115200; use `--baudrate` to override it and press `q` to quit.

To generate the POSIX graph, run this command from the `examples/spectrogram` directory:

```powershell
uv run create.py --runner posix
```

Then build the runner from the repository root:

```powershell
cmake --build posix_runner/build
```

Pass `-` to make the terminal display read its binary input from standard input, and connect the POSIX runner's standard output to it with a pipe:

```powershell
posix_runner\build\Debug\app.exe | uv run examples\tools\spectrogram_terminal.py -
```

This binary pipe requires PowerShell 7.4 or newer. With an older PowerShell version, run the same command in Command Prompt (`cmd.exe`) so that the binary stream is not converted to text. End the runner with `Ctrl+C`; when the runner exits normally, the display closes after reaching end-of-file.
