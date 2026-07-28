# Copyright (c) 2021-2026 Arm Limited. All rights reserved.

# Virtual Streaming Interface instance 1 Python script: Audio Output

import logging
import os
import wave


logger = logging.getLogger(__name__)

verbosity = logging.ERROR
logging.basicConfig(
    format="Py: %(name)s : [%(levelname)s]\t%(message)s", level=verbosity
)


# IRQ registers
IRQ_Status = 0

# Timer registers
Timer_Control = 0
Timer_Interval = 0

Timer_Control_Run_Msk = 1 << 0
Timer_Control_Periodic_Msk = 1 << 1
Timer_Control_Trig_IRQ_Msk = 1 << 2
Timer_Control_Trig_DMA_Msk = 1 << 3

# DMA registers
DMA_Control = 0

# User registers (must match vstream_audio_out.c)
CONTROL = 0
STATUS = 0
DEVICE = -1
FILENAME = ""
CHANNELS = 1
SAMPLE_RATE = 16000
SAMPLE_BITS = 16

CONTROL_ENABLE_Msk = 1 << 0
CONTROL_MODE_Msk = 1 << 1
CONTROL_CONTINUOUS_Msk = 1 << 2

STATUS_ACTIVE_Msk = 1 << 0
STATUS_DATA_Msk = 1 << 1
STATUS_EOS_Msk = 1 << 2
STATUS_FILE_NAME_Msk = 1 << 3
STATUS_FILE_VALID_Msk = 1 << 4

WAVE = None
Data = bytearray()


def _close_wave():
    global WAVE

    if WAVE is not None:
        WAVE.close()
        WAVE = None


def _filename_is_valid(filename):
    if not filename:
        return False

    directory = os.path.dirname(os.path.abspath(filename))
    if not os.path.isdir(directory):
        logger.error("Output directory does not exist: '%s'", directory)
        return False
    return True


def _open_wave(filename):
    global WAVE

    _close_wave()
    try:
        WAVE = wave.open(filename, "wb")
        WAVE.setnchannels(CHANNELS)
        WAVE.setsampwidth((SAMPLE_BITS + 7) // 8)
        WAVE.setframerate(SAMPLE_RATE)
    except (OSError, EOFError, wave.Error) as error:
        _close_wave()
        logger.error("Cannot create WAV file '%s': %s", filename, error)
        return False

    logger.info(
        "Created WAV '%s': %d channel(s), %d bits, %d Hz",
        filename,
        CHANNELS,
        SAMPLE_BITS,
        SAMPLE_RATE,
    )
    return True


def init():
    logger.info("init() called")


def rdIRQ():
    logger.debug("rdIRQ() -> 0x%08X", IRQ_Status)
    return IRQ_Status


def wrIRQ(value):
    global IRQ_Status

    IRQ_Status = value
    logger.debug("wrIRQ(0x%08X)", value)
    return value


def wrTimer(index, value):
    global Timer_Control, Timer_Interval

    if index == 0:
        Timer_Control = value
    elif index == 1:
        Timer_Interval = value
    logger.debug("wrTimer(%d, 0x%08X)", index, value)
    return value


def timerEvent():
    if (CONTROL & CONTROL_CONTINUOUS_Msk) == 0:
        wrCONTROL(CONTROL & ~CONTROL_ENABLE_Msk)


def wrDMA(index, value):
    global DMA_Control

    if index == 0:
        DMA_Control = value
    logger.debug("wrDMA(%d, 0x%08X)", index, value)
    return value


def rdDataDMA(size):
    # VSI1 is an output peripheral. Keep the callback for API completeness.
    logger.debug("rdDataDMA(%d) ignored by audio output", size)
    return bytearray(size)


def wrDataDMA(data, size):
    global STATUS, Data

    Data = bytearray(data[:size])
    if (STATUS & STATUS_ACTIVE_Msk) != 0 and WAVE is not None:
        WAVE.writeframesraw(Data)
        STATUS |= STATUS_DATA_Msk
    logger.debug("wrDataDMA(%d)", size)


def wrCONTROL(value):
    global CONTROL, STATUS

    enable_changed = ((value ^ CONTROL) & CONTROL_ENABLE_Msk) != 0
    if enable_changed:
        if (value & CONTROL_ENABLE_Msk) != 0:
            if (
                (STATUS & STATUS_FILE_VALID_Msk) != 0
                and _open_wave(FILENAME)
            ):
                STATUS |= STATUS_ACTIVE_Msk
                STATUS &= ~STATUS_EOS_Msk
            else:
                STATUS &= ~STATUS_ACTIVE_Msk
        else:
            STATUS &= ~STATUS_ACTIVE_Msk
            _close_wave()

    CONTROL = value


def rdSTATUS():
    global STATUS

    value = STATUS
    STATUS &= ~STATUS_DATA_Msk
    return value


def wrFILENAME(value):
    global FILENAME, STATUS

    if (STATUS & STATUS_FILE_NAME_Msk) != 0:
        STATUS &= ~(STATUS_FILE_NAME_Msk | STATUS_FILE_VALID_Msk)
        FILENAME = ""
        _close_wave()

    if value != 0:
        FILENAME += chr(value & 0xFF)
        return

    STATUS |= STATUS_FILE_NAME_Msk
    if _filename_is_valid(FILENAME):
        STATUS |= STATUS_FILE_VALID_Msk
    else:
        STATUS &= ~STATUS_FILE_VALID_Msk


def rdRegs(index):
    if index == 0:
        return CONTROL
    if index == 1:
        return rdSTATUS()
    if index == 2:
        return DEVICE
    if index == 3:
        return len(FILENAME)
    if index == 4:
        return CHANNELS
    if index == 5:
        return SAMPLE_RATE
    if index == 6:
        return SAMPLE_BITS
    return 0


def wrRegs(index, value):
    global DEVICE, CHANNELS, SAMPLE_RATE, SAMPLE_BITS

    if index == 0:
        wrCONTROL(value)
    elif index == 1:
        value = STATUS
    elif index == 2:
        DEVICE = value
    elif index == 3:
        wrFILENAME(value)
    elif index == 4:
        CHANNELS = value
    elif index == 5:
        SAMPLE_RATE = value
    elif index == 6:
        SAMPLE_BITS = value

    logger.debug("wrRegs(%d, 0x%08X)", index, value)
    return value
