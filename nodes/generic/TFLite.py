from cmsis_stream.cg.scheduler import GenericSink


class TFLite(GenericSink):
    def __init__(self,name,nbInputs=1,nbOutputs=1):
        GenericSink.__init__(self,name,selectors=["ack"])
        # Acknowledge event output to tell
        # producer that the network is ready
        self.addEventInput(nbInputs)
        self.addEventOutput(nbOutputs+1)
        self.addVariableArg(f"params->{name}")

    @property
    def needsHardware(self):
        return True



