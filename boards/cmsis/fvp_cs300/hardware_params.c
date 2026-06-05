#include "hardware_params.h"
#include "app_params.h"
#include "cmsis_os2.h"
#include "cmsis_vstream.h"
#include "datatypes.h"
#include "vstream_audio_in.h"

#ifndef MIC_SAMPLE_RATE
#error "MIC_SAMPLE_RATE must be defined by the application"
#endif

#ifndef MIC_CHANNELS
#error "MIC_CHANNELS must be defined by the application"
#endif

#ifndef MIC_BLOCK_SIZE
#error "MIC_BLOCK_SIZE must be defined by the application"
#endif

#ifndef MIC_SAMPLE_SIZE
#error "MIC_SAMPLE_SIZE must be defined by the application"
#endif


/* The number of blocks in audio buffer */
#define AUDIO_BUF_BLOCK_CNT     (2U)


#define vStream_AudioIn         (&Driver_vStreamAudioIn)

static uint8_t audio_buffer[MIC_BLOCK_SIZE * AUDIO_BUF_BLOCK_CNT]
    __attribute__((section(".bss.audio_dma"), aligned(32)));
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

    if (vStream_AudioIn->Initialize(audio_event_callback) != VSTREAM_OK) {
        (void)osEventFlagsDelete(audio_event);
        audio_event = NULL;
        return -3;
    }

    if (vStream_AudioIn->SetBuf(audio_buffer,
                                     sizeof(audio_buffer),
                                     MIC_BLOCK_SIZE) !=
        VSTREAM_OK) {
        (void)vStream_AudioIn->Uninitialize();
        (void)osEventFlagsDelete(audio_event);
        audio_event = NULL;
        return -4;
    }

    params->microphone_stream = (void*)vStream_AudioIn;
    params->microphone_event = audio_event;
    params->microphone_sample_rate = MIC_SAMPLE_RATE;
    params->microphone_num_channels = MIC_CHANNELS;
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
