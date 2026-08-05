from cmsis_stream.cg.scheduler import GenericNode,CType,F32,Q15,CStructType

_F32_STEREO = CStructType("sf32",8)
_Q15_STEREO = CStructType("sq15",4)

class InterleavedStereoToMono(GenericNode):
    def __init__(self,name,theType,ioLength):
        GenericNode.__init__(self,name,identified=False)
        if theType._id == F32:
            inputType = _F32_STEREO
            outputType = CType(F32)
        elif theType._id == Q15:
            inputType = _Q15_STEREO
            outputType = CType(Q15)

        self.addInput("i",inputType,ioLength)
        self.addOutput("o",outputType,ioLength)

    @property
    def typeName(self):
        return "InterleavedStereoToMono"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "generic"