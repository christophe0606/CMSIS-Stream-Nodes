# /// script
# requires-python = ">=3.10"
# dependencies = ["pyserial>=3.5"]
# ///

"""Display framed SpectrogramTextDisplay output from serial or stdin."""

import argparse
import math
import os
import shutil
import struct
import sys
import time
import zlib

import serial
from serial.tools import list_ports

if os.name == "nt":
    import msvcrt
else:
    import select
    import termios
    import tty


MAGIC = b"\xa5\x5a\xc3\x3cSPG1"
HEADER_SIZE = len(MAGIC) + 2 + 4
TRAILER_SIZE = 4
MAX_BINS = 4096


class FrameParser:
    """Extract valid frames while discarding startup logs and corrupt data."""

    def __init__(self):
        self.buffer = bytearray()

    def feed(self, data):
        self.buffer.extend(data)
        frames = []

        while True:
            marker = self.buffer.find(MAGIC)
            if marker < 0:
                keep = min(len(self.buffer), len(MAGIC) - 1)
                if keep:
                    del self.buffer[:-keep]
                else:
                    self.buffer.clear()
                break

            if marker:
                del self.buffer[:marker]

            if len(self.buffer) < HEADER_SIZE:
                break

            bin_count, sequence = struct.unpack_from("<HI", self.buffer, len(MAGIC))
            if not 0 < bin_count <= MAX_BINS:
                del self.buffer[0]
                continue

            frame_size = HEADER_SIZE + 4 * bin_count + TRAILER_SIZE
            if len(self.buffer) < frame_size:
                break

            checksum_offset = frame_size - TRAILER_SIZE
            expected_crc = struct.unpack_from("<I", self.buffer, checksum_offset)[0]
            actual_crc = zlib.crc32(self.buffer[len(MAGIC):checksum_offset])
            if actual_crc != expected_crc:
                del self.buffer[0]
                continue

            amplitudes = struct.unpack_from(
                f"<{bin_count}f", self.buffer, HEADER_SIZE
            )
            del self.buffer[:frame_size]

            if all(math.isfinite(value) for value in amplitudes):
                frames.append((sequence, amplitudes))

        return frames


class SpectrogramDisplay:
    def __init__(self, refresh_ms):
        self.refresh_seconds = refresh_ms / 1000.0
        self.next_refresh = 0.0
        self.width = 0
        self.height = 0
        self.started = False

    @staticmethod
    def color(amplitude):
        if amplitude < 0.5:
            return 46
        if amplitude < 0.7:
            return 208
        return 196

    def add(self, amplitudes):
        now = time.monotonic()
        if now < self.next_refresh:
            return
        self.next_refresh = now + self.refresh_seconds
        self.render(amplitudes)

    def render(self, amplitudes):
        terminal = shutil.get_terminal_size(fallback=(40, 20))
        width = max(1, terminal.columns)
        height = max(1, terminal.lines)

        prefix = ""
        if not self.started or width != self.width or height != self.height:
            self.width = width
            self.height = height
            prefix = "\x1b[2J\x1b[?25l\x1b[?7l"
            self.started = True

        output = [prefix]

        for row in range(height):
            output.append(f"\x1b[{row + 1};1H")
            current_color = None

            for column in range(width):
                bin_index = column * len(amplitudes) // width
                amplitude = min(1.0, max(0.0, amplitudes[bin_index]))
                bar_height = int(amplitude * height)
                color = None
                if row >= height - bar_height:
                    color = self.color(amplitude)

                if color != current_color:
                    output.append(
                        "\x1b[49m" if color is None else f"\x1b[48;5;{color}m"
                    )
                    current_color = color
                output.append(" ")

            output.append("\x1b[0m")

        output.append("\x1b[H")
        sys.stdout.write("".join(output))
        sys.stdout.flush()

    def close(self):
        sys.stdout.write("\x1b[0m\x1b[?7h\x1b[?25h\x1b[2J\x1b[H")
        sys.stdout.flush()


class QuitKey:
    """Provide a non-blocking q key on Windows and POSIX terminals."""

    def __init__(self):
        self.fd = None
        self.saved_settings = None

    def __enter__(self):
        if os.name != "nt" and sys.stdin.isatty():
            self.fd = sys.stdin.fileno()
            self.saved_settings = termios.tcgetattr(self.fd)
            tty.setcbreak(self.fd)
        return self

    def pressed(self):
        if os.name == "nt":
            while msvcrt.kbhit():
                if msvcrt.getwch().lower() == "q":
                    return True
            return False

        if self.fd is None:
            return False
        readable, _, _ = select.select([sys.stdin], [], [], 0)
        return bool(readable) and sys.stdin.read(1).lower() == "q"

    def __exit__(self, _exc_type, _exc_value, _traceback):
        if self.saved_settings is not None:
            termios.tcsetattr(self.fd, termios.TCSADRAIN, self.saved_settings)


def parse_args():
    parser = argparse.ArgumentParser(
        description="Display a binary spectrogram stream from serial or stdin."
    )
    parser.add_argument(
        "port",
        nargs="?",
        help=(
            "Serial port, or '-' to read a process stream from stdin; "
            "when omitted, the script asks which serial port to use"
        ),
    )
    parser.add_argument("--baudrate", type=int, default=115200)
    parser.add_argument(
        "--refresh-ms",
        type=int,
        default=40,
        help="Minimum interval between terminal redraws (default: 40)",
    )
    return parser.parse_args()


def choose_port():
    ports = list(list_ports.comports())
    if ports:
        print("Available serial ports:")
        for index, port in enumerate(ports, start=1):
            description = port.description or "Unknown device"
            print(f"  {index}. {port.device} - {description}")

        choice = input("Serial port number or name: ").strip()
        if choice.isdigit() and 1 <= int(choice) <= len(ports):
            return ports[int(choice) - 1].device
        if choice:
            return choice
    else:
        print("No serial ports were detected automatically.")

    while True:
        choice = input("Serial port (for example COM4): ").strip()
        if choice:
            return choice


def main():
    args = parse_args()
    parser = FrameParser()
    display = SpectrogramDisplay(args.refresh_ms)

    try:
        if args.port == "-":
            stream = sys.stdin.buffer
            read = getattr(stream, "read1", stream.read)
            while data := read(4096):
                for _sequence, amplitudes in parser.feed(data):
                    display.add(amplitudes)
        else:
            port = args.port or choose_port()
            print(f"Connecting to {port} at {args.baudrate} baud...")
            with serial.Serial(port, args.baudrate, timeout=0.05) as stream:
                with QuitKey() as quit_key:
                    while not quit_key.pressed():
                        data = stream.read(stream.in_waiting or 1)
                        for _sequence, amplitudes in parser.feed(data):
                            display.add(amplitudes)
    except KeyboardInterrupt:
        pass
    finally:
        display.close()


if __name__ == "__main__":
    main()
