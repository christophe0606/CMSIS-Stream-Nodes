from pathlib import Path
import struct
import sys
import wave

REPO_ROOT = Path(__file__).resolve().parents[2]
path_text = str(REPO_ROOT)
if REPO_ROOT.exists() and path_text not in sys.path:
    sys.path.insert(0, path_text)

from cmsis_stream.cg.scheduler import CType, F32, Graph, Q15
from examples.common.app import configure_app_from_args, mk_app
from nodes.generic import Convert, DebugSendToNetwork, DebugSink, DebugSource
from nodes.generic import Gain, KWS, MFCC, SlidingBuffer, DebugKWSClassify


SAMPLE_RATE = 16_000
FRAME_DURATION_MS = 20
TEST_DURATION_SECONDS = 1
TEST_AUDIO_START_SECONDS = 1
TEST_DURATION_FRAMES = TEST_DURATION_SECONDS * 1000 // FRAME_DURATION_MS
AUDIO_SAMPLES_PER_FRAME = SAMPLE_RATE * FRAME_DURATION_MS // 1000
AUDIO_SAMPLE_COUNT = SAMPLE_RATE * TEST_DURATION_SECONDS

MFCC_FEATURES = 10
NN_FEATURES = 49
WINDOW_DURATION_MS = 40
OVERLAP_DURATION_MS = 20
WINDOW_SAMPLES = SAMPLE_RATE * WINDOW_DURATION_MS // 1000
OVERLAP_SAMPLES = SAMPLE_RATE * OVERLAP_DURATION_MS // 1000
MFCC_OVERLAP = NN_FEATURES - 1


def write_if_changed(path, contents):
    if not path.exists() or path.read_text() != contents:
        path.write_text(contents, newline="\n")


def generate_debug_audio():
    wav_path = REPO_ROOT / "examples" / "assets" / "sample_audio.wav"
    with wave.open(str(wav_path), "rb") as wav_file:
        if wav_file.getnchannels() != 1:
            raise ValueError("sample_audio.wav must be mono")
        if wav_file.getsampwidth() != 2:
            raise ValueError("sample_audio.wav must contain 16-bit PCM samples")
        if wav_file.getframerate() != SAMPLE_RATE:
            raise ValueError(f"sample_audio.wav must use a {SAMPLE_RATE} Hz sample rate")
        wav_file.setpos(SAMPLE_RATE * TEST_AUDIO_START_SECONDS)
        raw_audio = wav_file.readframes(AUDIO_SAMPLE_COUNT)

    samples = struct.unpack(f"<{len(raw_audio) // 2}h", raw_audio)
    if len(samples) != AUDIO_SAMPLE_COUNT:
        raise ValueError("sample_audio.wav is shorter than the one-second test")

    rows = []
    for offset in range(0, len(samples), 12):
        rows.append("    " + ", ".join(str(value) for value in samples[offset : offset + 12]))

    header = f"""#pragma once

#include <stdint.h>

#define DEBUG_AUDIO_SAMPLE_COUNT {AUDIO_SAMPLE_COUNT}

#ifdef __cplusplus
extern "C" {{
#endif

extern const int16_t debug_audio_samples[DEBUG_AUDIO_SAMPLE_COUNT];

#ifdef __cplusplus
}}
#endif
"""
    source = """#include \"debug_audio.h\"

const int16_t debug_audio_samples[DEBUG_AUDIO_SAMPLE_COUNT] = {
""" + ",\n".join(rows) + "\n};\n"

    csrc = REPO_ROOT / "src" / "csrc"
    write_if_changed(csrc / "debug_audio.h", header)
    write_if_changed(csrc / "debug_audio.c", source)


generate_debug_audio()
config = configure_app_from_args()
graph = Graph()

q15 = CType(Q15)
f32 = CType(F32)

source = DebugSource("source", q15, AUDIO_SAMPLES_PER_FRAME)
gain = Gain("gain", q15, AUDIO_SAMPLES_PER_FRAME, 1)
to_f32 = Convert("to_f32", q15, f32, AUDIO_SAMPLES_PER_FRAME)
audio_window = SlidingBuffer("audio_window", f32, WINDOW_SAMPLES, OVERLAP_SAMPLES)
mfcc = MFCC("mfcc", WINDOW_SAMPLES, MFCC_FEATURES)
mfcc_window = SlidingBuffer(
    "mfcc_window",
    f32,
    MFCC_FEATURES * NN_FEATURES,
    MFCC_FEATURES * MFCC_OVERLAP,
)
send = DebugSendToNetwork("send", f32, MFCC_FEATURES * NN_FEATURES)
kws = KWS("kws")
classify = DebugKWSClassify("classify", params={"historyLength": 10})
sink = DebugSink("sink")

graph.connect(source.o, gain.i)
graph.connect(gain.o, to_f32.i)
graph.connect(to_f32.o, audio_window.i)
graph.connect(audio_window.o, mfcc.i)
graph.connect(mfcc.o, mfcc_window.i)
graph.connect(mfcc_window.o, send.i)
graph.connect(send["oev0"], kws["iev0"])
graph.connect(kws["oev1"], classify["iev0"])
graph.connect(classify["oev0"], sink["iev0"])

sample_type = CType(Q15)
mic_channels = 2
mic_frames_per_buffer = 0 # only used for posix portaudio

mk_app(
    graph,
    globals={
        "TEST_DURATION_FRAMES": TEST_DURATION_FRAMES,
        "MIC_SAMPLE_RATE": SAMPLE_RATE,
        "MIC_CHANNELS": mic_channels,
        "MIC_FRAMES_PER_BUFFER": 0,
        "MIC_BLOCK_SIZE": AUDIO_SAMPLES_PER_FRAME * sample_type.bytes * mic_channels,
        "MIC_FRAMES_PER_BUFFER": mic_frames_per_buffer,
        "MIC_SAMPLE_SIZE" :  sample_type.bytes * 8,
        "DEBUG_AUDIO": True,
        "VSI0_FILE_PATH": '"examples/assets/sample_audio.wav"',
    },
    config=config,
)
