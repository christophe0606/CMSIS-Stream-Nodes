from cmsis_stream.cg.scheduler import GenericSource


class DebugSource(GenericSource):
    def __init__(self, name, theType, outputLength):
        GenericSource.__init__(self, name, identified=True)
        self.addOutput("o", theType, outputLength)

    @property
    def typeName(self):
        return "DebugSource"
