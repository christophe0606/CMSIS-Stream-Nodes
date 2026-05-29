# README.md

`runner_common` us an attempt at factoring out all files that are common between different runners.

But due to the way CMSIS Toolbox is working, the application specific files are generated and customized in `RTE` folder.

Those files are the same as the ones in `config` folder that is used by other runtimes.

As consequence, the `RTE` files are just including the files from `config` folder to avoid code duplication.


