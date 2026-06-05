from pathlib import Path
import sys

REPO_ROOT = Path(__file__).resolve().parents[2]
path_text = str(REPO_ROOT)
if REPO_ROOT.exists() and path_text not in sys.path:
    sys.path.insert(0, path_text)

from cmsis_stream.cg.scheduler import Graph,CType,SINT16
from examples.common.app import configure_app_from_args, mk_app
from nodes.generic import DebugSink, DebugSource  
from nodes.posix import MicrophoneSource

config = configure_app_from_args()

the_graph = Graph()

sample_type = CType(SINT16)
mic_sample_rate = 16000
block_size = (mic_sample_rate // 10)  # 100ms of audio
mic_channels = 1
mic_frames_per_buffer = 0 # only used for posix portaudio
src_value = 2

#src = DebugSource("src", sample_type, block_size,params={"value": ("APP_SRC_VALUE", sample_type)})
src = MicrophoneSource("src", sample_type, block_size)
sink = DebugSink("sink", sample_type, block_size)

the_graph.connect(src.o, sink.i)

mk_app(
    the_graph,
    globals={
        "MIC_BLOCK_SIZE": block_size* sample_type.bytes,
        "MIC_SAMPLE_RATE": mic_sample_rate,
        "MIC_CHANNELS": mic_channels,
        "MIC_FRAMES_PER_BUFFER": mic_frames_per_buffer,
        "MIC_SAMPLE_SIZE" :  sample_type.bytes * 8,
        "APP_SRC_VALUE": src_value,
    },
    config=config,
    debug_limit=10
)
