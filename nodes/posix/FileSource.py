from cmsis_stream.cg.scheduler import GenericSource


class FileSource(GenericSource):
    def __init__(self, name, theType, outputLength):
        GenericSource.__init__(self, name, identified=True)
        self.addOutput("o", theType, outputLength)
        self.addVariableArg(f"params->{name}")

    @property
    def needsHardware(self):
        return False

    @property
    def folder(self):
        return "posix"

    @property
    def typeName(self):
        return "FileSource"
