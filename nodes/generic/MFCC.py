from cmsis_stream.cg.scheduler import GenericNode,CType,F32

class MFCC(GenericNode):
    def __init__(self,name,inLength,outLength):
        GenericNode.__init__(self,name)
        self.addInput("i",CType(F32),inLength)
        self.addOutput("o",CType(F32),outLength)

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "generic"
    
    @property
    def typeName(self):
        return "MFCC"