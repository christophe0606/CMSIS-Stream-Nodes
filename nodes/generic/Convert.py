from cmsis_stream.cg.scheduler import GenericNode


class Convert(GenericNode):
    def __init__(self,name,srcType,dstType,outLength):
        GenericNode.__init__(self,name)
        self.addInput("i",srcType,outLength)
        self.addOutput("o",dstType,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Convert"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "generic"