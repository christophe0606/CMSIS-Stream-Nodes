from cmsis_stream.cg.scheduler import GenericSink


class DebugSink(GenericSink):
    def __init__(self, name, theType, inputLength):
        GenericSink.__init__(self, name, identified=True)
        self.addInput("i", theType, inputLength)

    @property
    def typeName(self):
        return "DebugSink"
