from cmsis_stream.cg.scheduler import GenericSource


class _CType:
    def __init__(self, ctype):
        self.ctype = ctype


_CONST_CHAR_PTR = _CType("const char *")


def _c_string(value):
    escaped = str(value).replace("\\", "\\\\").replace('"', '\\"')
    return f'"{escaped}"'


def _wav_type_params(theType):
    ctype = getattr(theType, "ctype", None)
    if ctype == "float":
        return {
            "num_channels": 1,
            "sample_format": 4,
            "channel_format": 0,
        }
    if ctype == "sf32":
        return {
            "num_channels": 2,
            "sample_format": 4,
            "channel_format": 0,
        }
    if ctype == "q15_t":
        return {
            "num_channels": 1,
            "sample_format": 2,
            "channel_format": 0,
        }
    if ctype == "sq15":
        return {
            "num_channels": 2,
            "sample_format": 2,
            "channel_format": 0,
        }
    raise ValueError(f"Unsupported WAV node type: {ctype!r}")


class WavSource(GenericSource):
    def __init__(self, name, theType, outputLength, path, params=None):
        GenericSource.__init__(self, name, identified=True)
        self.params = {
            "path": (_c_string(path), _CONST_CHAR_PTR),
            **_wav_type_params(theType),
        }
        if params:
            self.params.update(params)
        self.addOutput("o", theType, outputLength)
        self.addVariableArg(f"params->{name}")

    @property
    def needsHardware(self):
        return False

    @property
    def folder(self):
        return "posix"

    @property
    def typeName(self):
        return "WavSource"
