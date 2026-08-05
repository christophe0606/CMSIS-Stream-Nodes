from cmsis_stream.cg.scheduler import BaseNode


class SpectrogramTextDisplay(BaseNode):
    def __init__(self,name):
        BaseNode.__init__(self,name)
        self.addEventInput(1)


    @property
    def folder(self):
        """The folder containing the C++ class implementing this node"""
        return "generic"

    @property
    def typeName(self):
        """The name of the C++ class implementing this node"""
        return "SpectrogramTextDisplay"