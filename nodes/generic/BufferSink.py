from cmsis_stream.cg.scheduler import GenericSink


class BufferSink(GenericSink):
    def __init__(self, name, theType, inputLength):
        GenericSink.__init__(self, name, identified=True)
        self.addInput("i", theType, inputLength)
        self.addVariableArg(f"params->{name}")

    @property
    def needsHardware(self):
        return False

    @property 
    def folder(self):
        return "generic"

    @property
    def typeName(self):
        return "BufferSink"
