from pathlib import Path
import sys

REPO_ROOT = Path(__file__).resolve().parents[2]
path_text = str(REPO_ROOT)
if REPO_ROOT.exists() and path_text not in sys.path:
    sys.path.insert(0, path_text)

from cmsis_stream.cg.scheduler import Graph,CType,SINT16
from examples.common.app import configure_app_from_args, mk_app
from nodes.generic import DebugSink, DebugSource, MicrophoneSource

config = configure_app_from_args()

the_graph = Graph()

# Mono sample datatype for KWS application
sample_type = CType(SINT16)
AUDIO_PACKET_DURATION = 20 # ms 
OVERLAP_DURATION = 20 
WINDOWS_DURATION = 40 

mic_sample_rate = 16000
# Stereo source for microphone hardware, but only one channel is used for the KWS application
mic_channels = 2
mic_frames_per_buffer = 0 # only used for posix portaudio



NB_AUDIO_SAMPLES = int(1e-3 * AUDIO_PACKET_DURATION * mic_sample_rate)
NB_OVERLAP_SAMPLES = int(1e-3 * OVERLAP_DURATION * mic_sample_rate)
NB_WINDOW_SAMPLES = int(1e-3 * WINDOWS_DURATION * mic_sample_rate)


#src = DebugSource("src", sample_type, block_size,params={"value": ("APP_SRC_VALUE", sample_type)})
src = MicrophoneSource("src", sample_type, NB_AUDIO_SAMPLES)
sink = DebugSink("sink", sample_type, NB_AUDIO_SAMPLES)

the_graph.connect(src.o, sink.i)

mk_app(
    the_graph,
    globals={
        "MIC_BLOCK_SIZE": NB_AUDIO_SAMPLES * sample_type.bytes,
        "MIC_SAMPLE_RATE": mic_sample_rate,
        "MIC_CHANNELS": mic_channels,
        "MIC_FRAMES_PER_BUFFER": mic_frames_per_buffer,
        "MIC_SAMPLE_SIZE" :  sample_type.bytes * 8,
        "VSI0_FILE_PATH": '"examples/assets/sample_audio.wav"',
    },
    config=config,
    debug_limit=10
)
