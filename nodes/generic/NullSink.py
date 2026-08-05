from cmsis_stream.cg.scheduler import GenericSink


class NullSink(GenericSink):
    def __init__(self,name,theType,outLength):
        GenericSink.__init__(self,name,identified=False)
        # Stereo output
        self.addInput("i",theType,outLength)
        self.addEventInput()
        self.addEventOutput()

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "NullSink"
    
    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "generic"