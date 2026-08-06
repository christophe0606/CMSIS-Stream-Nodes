from cmsis_stream.cg.scheduler import GenericSink


class DebugSendToNetwork(GenericSink):
    def __init__(self, name, theType, inputLength):
        GenericSink.__init__(self, name, identified=True)
        self.addInput("i", theType, inputLength)
        self.addEventOutput()

    @property
    def folder(self):
        return "generic"

    @property
    def typeName(self):
        return "DebugSendToNetwork"
