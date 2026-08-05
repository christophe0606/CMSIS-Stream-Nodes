from cmsis_stream.cg.scheduler import GenericNode,CType,F32

def _is_power_of_two(n: int) -> bool:
    """Return True if n is a positive power of 2, otherwise False."""
    return n > 0 and (n & (n - 1)) == 0

class Hanning(GenericNode):
    def __init__(self,name,inLength,outLength):
        GenericNode.__init__(self,name,identified=False)
        assert _is_power_of_two(outLength), "Output length must be a power of two"
        self.addInput("i",CType(F32),inLength)
        self.addOutput("o",CType(F32),outLength)

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Hanning"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "generic"