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
block_size = 64
mic_sample_rate = 16000
mic_channels = 1
mic_frames_per_buffer = 0
src_value = 2

#src = DebugSource("src", sample_type, block_size,params={"value": ("APP_SRC_VALUE", sample_type)})
src = MicrophoneSource("src", sample_type, block_size)
sink = DebugSink("sink", sample_type, block_size)

the_graph.connect(src.o, sink.i)

mk_app(
    the_graph,
    globals={
        "APP_BLOCK_SIZE": block_size,
        "APP_MIC_SAMPLE_RATE": mic_sample_rate,
        "APP_MIC_CHANNELS": mic_channels,
        "APP_MIC_FRAMES_PER_BUFFER": mic_frames_per_buffer,
        "APP_SRC_VALUE": src_value,
    },
    config=config,
    debug_limit=10
)
