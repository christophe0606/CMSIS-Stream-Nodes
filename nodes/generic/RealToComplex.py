from cmsis_stream.cg.scheduler import GenericNode,F32,Q15,CStructType,CType

_F32_COMPLEX = CStructType("cf32",8)
_Q15_COMPLEX = CStructType("cq15",4)


class RealToComplex(GenericNode):
    def __init__(self,name,theType,outLength):
        GenericNode.__init__(self,name,identified=False)
        if theType._id == F32:
            inputType = CType(F32)
            outputType = _F32_COMPLEX
        elif theType._id == Q15:
            inputType = CType(Q15)
            outputType = _Q15_COMPLEX
        else:
            raise ValueError("Unsupported type for RealToComplex: {}".format(theType))
        self.addInput("i",inputType,outLength)
        self.addOutput("o",outputType,outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "RealToComplex"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "generic"