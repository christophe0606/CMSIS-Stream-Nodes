from cmsis_stream.cg.scheduler import GenericNode,CType,F32

class SlidingBuffer(GenericNode):

    def __init__(self,name,theType,length,overlap):
        GenericNode.__init__(self,name)
        self._length = length 
        self._overlap = overlap 
        self.addInput("i",theType,length-overlap)
        self.addOutput("o",theType,length)
    
    def ioTemplate(self):
        """ioTemplate is different for window
        """
        theType=self._inputs[self.inputNames[0]].ctype  
        ios="%s,%d,%d" % (theType,self._length,self._overlap)
        return(self._bracket(ios))

    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "generic"

    @property
    def typeName(self):
        return "SlidingBuffer"