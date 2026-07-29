# Zephyr runner

## Visual Studio Code configuration

Check that `.vscode/settings.json` does not contain the following setting:

```json
"cmake.sourceDirectory": "${workspaceFolder}/posix_runner"
```

It conflicts with Zephyr builds performed through the CMSIS Toolbox extension.

## CMSIS-Stream module

Add CMSIS-Stream to the projects in your `west.yml` file:

```yaml
- name: cmsisstream
  url: https://github.com/ARM-software/CMSIS-Stream
  revision: main
  path: modules/lib/cmsisstream
```

Then build using Visual Studio Code and the CMSIS Toolbox extension.

On FVP, use the corresponding `fvp_config.txt` fle provided in the board folder (for instance `boards\zephyr\fvp_cs300\fvp_config.txt`).

On Windows, and dependign on the shell used, you may need to use `winpty` to call the FVP otherwise some part of the output may not be flushed.

[Back to the project README](../README.md)

