# Python Node Descriptions

Python node descriptions may need to choose different generated C++ include
paths or C++ type names depending on the selected runner and board.

The application generator exposes that selection through the common app
configuration in `examples.common.app`. A node descriptor can load the current
configuration and use `config.runner` and `config.board` to decide what its
`folder`, `typeName`, constructor arguments, or other generated properties
should be.

In `create.py` for an app, configure the application once before constructing nodes:

```python
from examples.common.app import configure_app_from_args, mk_app


config = configure_app_from_args()

# Construct graph nodes after configure_app_from_args().

mk_app(the_graph, config=config)
```

A node description should only read the already-selected configuration:

```python
from cmsis_stream.cg.scheduler import GenericSource
from examples.common.app import get_app_config


class MySource(GenericSource):
    def __init__(self, name, theType, outputLength):
        GenericSource.__init__(self, name, identified=True)
        self.config = get_app_config()
        self.addOutput("o", theType, outputLength)

    @property
    def folder(self):
        if self.config.runner == "zephyr":
            return "zephyr"
        if self.config.board == "AlifE7":
            return "alif"
        return "generic"

    @property
    def typeName(self):
        if self.config.runner == "posix" and self.config.board == "Windows":
            return "WindowsMySource"
        return "MySource"
```

The mapping from `(runner, board)` to `folder` and `typeName` is intentionally
owned by each node descriptor. Some nodes may only care about the runner,
others may care about the board, and many generic nodes can ignore both.

# Node parameters

The node constructor may use
```python
self.addVariableArg(f"params->{name}")
```

to add a parameter argument for use in the C++ constructor.
This shoudl be used only when the Python constructor has a params parameter taking a dictionnary as value or when the node has the property `needsHardware` returning `True`. It implies the node must have access to hardware parameters