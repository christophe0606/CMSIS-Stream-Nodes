from cmsis_stream.cg.scheduler import GenericSink

class KWSClassify(GenericSink):
    def __init__(self,name, params=None):
        GenericSink.__init__(self,name)
        self.params = params or {}
        self.addEventInput(1)
        self.addEventOutput(1)
        self.addVariableArg(f"params->{name}")

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "generic"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "KWSClassify"