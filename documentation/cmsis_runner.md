# CMSIS runner

## CMSIS-Stream pack

The CMSIS pack for CMSIS-Stream has not yet been upstreamed, but you can reference it from your `csolution` file:

```yaml
- pack: ARM::CMSIS-STREAM
  path: ../../CMSIS-Stream
```

Set `path` to the location of your CMSIS-Stream repository clone. Then build using Visual Studio Code and the CMSIS Toolbox extension.

## Audio on FVP

Audio on FVP uses the `AVH_FVP` pack. It is not yet available from the pack repository, so clone the [AVH repository](https://github.com/ARM-software/AVH) and reference the pack from your `csolution` file:

```yaml
- pack: ARM::AVH_FVP
  path: ../../AVH
```

Set `path` to the location of your AVH repository clone.

On FVP, use the corresponding `fvp_config.txt` file provided in the board folder (for instance `boards\cmsis\fvp_cs300\fvp_config.txt`).

On Windows, depending on the shell, you may need to invoke the FVP through `winpty`; otherwise, some output may not be flushed.

[Back to the project README](../README.md)
