from cmsis_stream.cg.scheduler import GenericNode


class CFFT(GenericNode):
    def __init__(self,name,theType,outLength):
        GenericNode.__init__(self,name,identified=False)
        self.addInput("i",theType,outLength)
        self.addOutput("o",theType,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "CFFT"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "generic"