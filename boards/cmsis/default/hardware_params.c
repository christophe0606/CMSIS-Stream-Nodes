#include "hardware_params.h"
#include "app_params.h"
#include "cmsis_os2.h"
#include "cmsis_vstream.h"
#include "datatypes.h"

#ifndef APP_MIC_SAMPLE_RATE
#error "APP_MIC_SAMPLE_RATE must be defined by the application"
#endif

#ifndef APP_MIC_CHANNELS
#error "APP_MIC_CHANNELS must be defined by the application"
#endif

#ifndef APP_BLOCK_SIZE
#error "APP_BLOCK_SIZE must be defined by the application"
#endif

#define AUDIO_SOURCE_BLOCK_COUNT (2U)

extern vStreamDriver_t Driver_vStreamAudioIn;

static q15_t audio_buffer[AUDIO_SOURCE_BLOCK_COUNT * APP_BLOCK_SIZE * APP_MIC_CHANNELS];
static osEventFlagsId_t audio_event = NULL;

static void audio_event_callback(uint32_t event)
{
    if (audio_event == NULL) {
        return;
    }

    uint32_t flags = 0U;
    if ((event & VSTREAM_EVENT_DATA) != 0U) {
        flags |= HARDWARE_MICROPHONE_FRAME_EVENT;
    }
    if ((event & VSTREAM_EVENT_OVERFLOW) != 0U) {
        flags |= HARDWARE_MICROPHONE_OVERFLOW_EVENT;
    }
    if (flags != 0U) {
        osEventFlagsSet(audio_event, flags);
    }
}

int hardware_params_init(HardwareParams *params)
{
    if (params == 0) {
        return -1;
    }

    audio_event = osEventFlagsNew(NULL);
    if (audio_event == NULL) {
        return -2;
    }

    if (Driver_vStreamAudioIn.Initialize(audio_event_callback) != VSTREAM_OK) {
        (void)osEventFlagsDelete(audio_event);
        audio_event = NULL;
        return -3;
    }

    if (Driver_vStreamAudioIn.SetBuf(audio_buffer,
                                     sizeof(audio_buffer),
                                     APP_BLOCK_SIZE * APP_MIC_CHANNELS * sizeof(q15_t)) !=
        VSTREAM_OK) {
        (void)Driver_vStreamAudioIn.Uninitialize();
        (void)osEventFlagsDelete(audio_event);
        audio_event = NULL;
        return -4;
    }

    params->microphone_stream = &Driver_vStreamAudioIn;
    params->microphone_event = audio_event;
    params->microphone_sample_rate = APP_MIC_SAMPLE_RATE;
    params->microphone_num_channels = APP_MIC_CHANNELS;
    return 0;
}

void hardware_params_uninit(HardwareParams *params)
{
    if (params == 0) {
        return;
    }

    if (params->microphone_stream != NULL) {
        vStreamDriver_t *driver = (vStreamDriver_t *)params->microphone_stream;
        (void)driver->Stop();
        (void)driver->Uninitialize();
    }

    if (audio_event != NULL) {
        (void)osEventFlagsDelete(audio_event);
        audio_event = NULL;
    }

    params->microphone_stream = NULL;
    params->microphone_event = NULL;
}

int hardware_audio_pause(HardwareParams *params)
{
    if ((params == 0) || (params->microphone_stream == NULL)) {
        return -1;
    }
    vStreamDriver_t *driver = (vStreamDriver_t *)params->microphone_stream;
    return driver->Stop() == VSTREAM_OK ? 0 : -1;
}

int hardware_audio_resume(HardwareParams *params)
{
    if ((params == 0) || (params->microphone_stream == NULL)) {
        return -1;
    }
    vStreamDriver_t *driver = (vStreamDriver_t *)params->microphone_stream;
    return driver->Start(VSTREAM_MODE_CONTINUOUS) == VSTREAM_OK ? 0 : -1;
}
