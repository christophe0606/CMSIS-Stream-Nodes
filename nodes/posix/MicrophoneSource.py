from cmsis_stream.cg.scheduler import GenericSource


def _microphone_channels(theType):
    ctype = getattr(theType, "ctype", None)
    if ctype in ("sf32", "sq15"):
        return 2
    return 1


class MicrophoneSource(GenericSource):
    def __init__(self, name, theType, outputLength, params=None):
        GenericSource.__init__(self, name, identified=True)
        self.params = {
            "num_channels": _microphone_channels(theType),
        }
        if params:
            self.params.update(params)
        self.addOutput("o", theType, outputLength)
        self.addVariableArg(f"params->{name}")

    @property
    def needsHardware(self):
        return True

    @property
    def folder(self):
        return "posix"

    @property
    def typeName(self):
        return "MicrophoneSource"
