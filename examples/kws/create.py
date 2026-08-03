from pathlib import Path
import sys

REPO_ROOT = Path(__file__).resolve().parents[2]
path_text = str(REPO_ROOT)
if REPO_ROOT.exists() and path_text not in sys.path:
    sys.path.insert(0, path_text)

from cmsis_stream.cg.scheduler import Graph,CType,SINT16,F32,Q15
from examples.common.app import configure_app_from_args, mk_app
from nodes.generic import MicrophoneSource, Convert, SlidingBuffer, SendToNetwork
from nodes.generic import MFCC, KWS, KWSClassify

config = configure_app_from_args()

the_graph = Graph()
    
# Mono sample datatype for KWS application
sample_type = CType(Q15)
AUDIO_PACKET_DURATION = 20 # ms 
OVERLAP_DURATION = 20 
WINDOWS_DURATION = 40 

mic_sample_rate = 16000
# Stereo source for microphone hardware, but only one channel is used for the KWS application
mic_channels = 1
mic_frames_per_buffer = 0 # only used for posix portaudio



NB_AUDIO_SAMPLES = int(1e-3 * AUDIO_PACKET_DURATION * mic_sample_rate)
NB_OVERLAP_SAMPLES = int(1e-3 * OVERLAP_DURATION * mic_sample_rate)
NB_WINDOW_SAMPLES = int(1e-3 * WINDOWS_DURATION * mic_sample_rate)


MFCC_FEATURES = 10
NN_FEATURES = 49

# Every new "audio" block of 20ms a new full tensor input is generated
# If it is too often, the overlap can be decreased
MFCC_OVERLAP = NN_FEATURES-1

NB = NB_AUDIO_SAMPLES
Q15_SCALAR = CType(Q15)
F32_SCALAR = CType(F32)

src = MicrophoneSource("src", sample_type, NB_AUDIO_SAMPLES)
to_f32 = Convert("to_f32",Q15_SCALAR,F32_SCALAR,NB)

audioWin=SlidingBuffer("audioWin",CType(F32),NB_WINDOW_SAMPLES,NB_OVERLAP_SAMPLES)
mfcc=MFCC("mfcc",NB_WINDOW_SAMPLES,MFCC_FEATURES)

mfccWin=SlidingBuffer("mfccWin",CType(F32),MFCC_FEATURES*NN_FEATURES,MFCC_FEATURES*MFCC_OVERLAP)

send = SendToNetwork("send",F32_SCALAR,MFCC_FEATURES*NN_FEATURES)

kws = KWS("kws")

classify = KWSClassify("classify",
                       params={"historyLength":10})

the_graph.connect(src.o,to_f32.i)
the_graph.connect(to_f32.o,audioWin.i)
the_graph.connect(audioWin.o,mfcc.i)
the_graph.connect(mfcc.o,mfccWin.i)
the_graph.connect(mfccWin.o,send.i)

the_graph.connect(send["oev0"],kws["iev0"])
the_graph.connect(kws["oev0"],send["iev0"])
the_graph.connect(kws["oev1"],classify["iev0"])


mk_app(
    the_graph,
    globals={
        "MIC_BLOCK_SIZE": NB_AUDIO_SAMPLES * sample_type.bytes * mic_channels,
        "MIC_SAMPLE_RATE": mic_sample_rate,
        "MIC_CHANNELS": mic_channels,
        "MIC_FRAMES_PER_BUFFER": mic_frames_per_buffer,
        "MIC_SAMPLE_SIZE" :  sample_type.bytes * 8,
        "VSI0_FILE_PATH": '"examples/assets/sample_audio.wav"',
    },
    config=config)
