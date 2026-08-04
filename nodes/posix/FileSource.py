from cmsis_stream.cg.scheduler import GenericSource
from cmsis_stream.cg.types import CGStaticType

class CStringType(CGStaticType):
    def __init__(self):
        self.ctype = "const char *"

_CONST_CHAR_PTR = CStringType()


class FileSource(GenericSource):
    def __init__(self, name, theType, outputLength, path):
        GenericSource.__init__(self, name, identified=True)
        self.params = {"path": path,"mode":0}
        
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
