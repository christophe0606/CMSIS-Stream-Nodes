from cmsis_stream.cg.scheduler import BaseNode


class DebugKWSClassify(BaseNode):
    def __init__(self, name, params=None):
        BaseNode.__init__(self, name, identified=False)
        self.addEventInput()
        self.addEventOutput()

    @property
    def folder(self):
        return "generic"

    @property
    def typeName(self):
        return "DebugKWSClassify"
