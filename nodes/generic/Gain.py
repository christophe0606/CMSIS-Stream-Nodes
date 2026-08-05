from cmsis_stream.cg.scheduler import GenericNode


class Gain(GenericNode):
    def __init__(self,name,theType,outLength,gain=1.0):
        GenericNode.__init__(self,name,identified=False)
        self.params = {"gain": float(gain)}
        self.addInput("i",theType,outLength)
        self.addOutput("o",theType,outLength)
        self.addVariableArg(f"params->{name}")

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Gain"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "generic"