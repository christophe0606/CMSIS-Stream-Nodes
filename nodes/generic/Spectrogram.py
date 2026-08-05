from cmsis_stream.cg.scheduler import GenericSink,CStructType

_F32_COMPLEX = CStructType("cf32",8)


class Spectrogram(GenericSink):
    def __init__(self,name,nbSamples):
        GenericSink.__init__(self,name,identified=False)
        self.addInput("i",_F32_COMPLEX,nbSamples)
        self.addEventOutput()

    @property
    def folder(self):
        """The folder where the C++ implementation of this node is located"""
        return "generic"
    
    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "Spectrogram"