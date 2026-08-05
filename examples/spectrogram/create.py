from pathlib import Path
import sys

REPO_ROOT = Path(__file__).resolve().parents[2]
path_text = str(REPO_ROOT)
if REPO_ROOT.exists() and path_text not in sys.path:
    sys.path.insert(0, path_text)

from cmsis_stream.cg.scheduler import CStructType, Graph,CType,Q15,F32
from examples.common.app import configure_app_from_args, mk_app
from nodes.generic import MicrophoneSource,Gain,Convert,SlidingBuffer,Hanning
from nodes.generic import RealToComplex,CFFT
from nodes.generic import Spectrogram, SpectrogramTextDisplay,InterleavedStereoToMono

config = configure_app_from_args()

the_graph = Graph()

Q15_SCALAR = CType(Q15)
Q15_STEREO = CStructType("sq15",4)

F32_SCALAR = CType(F32)
F32_COMPLEX = CStructType("cf32",8)
F32_STEREO = CStructType("sf32",8)


SAMPLING_FREQ_HZ = 16000
AUDIO_PACKET_DURATION = 20 # ms (320 samples)
OVERLAP_DURATION = AUDIO_PACKET_DURATION 
WINDOWS_DURATION = 2*AUDIO_PACKET_DURATION 

NB_AUDIO_SAMPLES = int(1e-3 * AUDIO_PACKET_DURATION * SAMPLING_FREQ_HZ)
NB_OVERLAP_SAMPLES = int(1e-3 * OVERLAP_DURATION * SAMPLING_FREQ_HZ)
NB_WINDOW_SAMPLES = int(1e-3 * WINDOWS_DURATION * SAMPLING_FREQ_HZ)
    
FFT_SIZE = 1024 # 512

    
mic_sample_rate = SAMPLING_FREQ_HZ
if config.board == "AlifE7":
   mic_channels = 2
else:
   mic_channels = 1
mic_frames_per_buffer = 0 # only used for posix portaudio

NB = NB_AUDIO_SAMPLES

#src = DebugSource("src", sample_type, block_size,params={"value": ("APP_SRC_VALUE", sample_type)})
if mic_channels == 2:
   src = MicrophoneSource("src", Q15_STEREO, NB)
   to_f32 = Convert("to_f32",Q15_STEREO,F32_STEREO,NB)
   to_mono = InterleavedStereoToMono("to_mono",F32_SCALAR,NB)
else:
   src = MicrophoneSource("src", Q15_SCALAR, NB)
   to_f32 = Convert("to_f32",Q15_SCALAR,F32_SCALAR,NB)
gain = Gain("gain",F32_SCALAR,NB,2)
audioWin=SlidingBuffer("audioWin",CType(F32),NB_WINDOW_SAMPLES,NB_OVERLAP_SAMPLES)
win = Hanning("winLeft",NB_WINDOW_SAMPLES,FFT_SIZE)
to_complex = RealToComplex("toComplex",CType(F32),FFT_SIZE)
fft= CFFT("fft",F32_COMPLEX,FFT_SIZE)
spectrogram = Spectrogram("spectrogram",FFT_SIZE)
display = SpectrogramTextDisplay("display")

if mic_channels == 2:
    the_graph.connect(src.o,to_f32.i)
    the_graph.connect(to_f32.o,to_mono.i)
    the_graph.connect(to_mono.o,gain.i)
else:
   the_graph.connect(src.o,to_f32.i)
   the_graph.connect(to_f32.o,gain.i)

the_graph.connect(gain.o,audioWin.i)
the_graph.connect(audioWin.o,win.i)
the_graph.connect(win.o,to_complex.i)
the_graph.connect(to_complex.o,fft.i)
the_graph.connect(fft.o,spectrogram.i)
the_graph.connect(spectrogram["oev0"],display["iev0"])

mk_app(
    the_graph,
    globals={
        "MIC_BLOCK_SIZE": NB_AUDIO_SAMPLES * Q15_SCALAR.bytes * mic_channels,
        "MIC_SAMPLE_RATE": mic_sample_rate,
        "MIC_CHANNELS": mic_channels,
        "MIC_FRAMES_PER_BUFFER": mic_frames_per_buffer,
        "MIC_SAMPLE_SIZE" :  Q15_SCALAR.bytes * 8,
        "CONFIG_NB_BINS" : 80,
        "VSI0_FILE_PATH": '"examples/assets/sample_audio.wav"',
    },
    config=config
)
