from cmsis_stream.cg.scheduler import GenericSink

from .WavSource import _CONST_CHAR_PTR, _c_string, _wav_type_params


class WavSink(GenericSink):
    def __init__(
        self,
        name,
        theType,
        inputLength,
        path,
        sample_rate,
        params=None,
    ):
        GenericSink.__init__(self, name, identified=True)
        self.params = {
            "path": (_c_string(path), _CONST_CHAR_PTR),
            "sample_rate": int(sample_rate),
            **_wav_type_params(theType),
        }
        if params:
            self.params.update(params)
        self.addInput("i", theType, inputLength)
        self.addVariableArg(f"params->{name}")

    @property
    def needsHardware(self):
        return False

    @property
    def folder(self):
        return "posix"

    @property
    def typeName(self):
        return "WavSink"
