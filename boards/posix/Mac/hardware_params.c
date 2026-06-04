#include "hardware_params.h"
#include "app_params.h"

#include <string.h>

#if defined(CMSIS_STREAM_NODES_USE_PORTAUDIO)
#include <portaudio.h>
#endif

// Typical default: 16000
#if !defined(APP_MIC_SAMPLE_RATE)
#error "APP_MIC_SAMPLE_RATE must be defined by the application"
#endif

// Typical default: 1
#if !defined(APP_MIC_CHANNELS)
#error "APP_MIC_CHANNELS must be defined by the application"
#endif

// Typical default: 0
#if !defined(APP_MIC_FRAMES_PER_BUFFER)
#error "APP_MIC_FRAMES_PER_BUFFER must be defined by the application"
#endif

int hardware_params_init(HardwareParams *params)
{
    if (params == 0) {
        return -1;
    }
    memset(params, 0, sizeof(*params));
    params->microphone_sample_rate = APP_MIC_SAMPLE_RATE;
    params->microphone_num_channels = APP_MIC_CHANNELS;

#if defined(CMSIS_STREAM_NODES_USE_PORTAUDIO)
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        return (int)err;
    }

    PaDeviceIndex device = Pa_GetDefaultInputDevice();
    if (device == paNoDevice) {
        Pa_Terminate();
        return -2;
    }

    const PaDeviceInfo *device_info = Pa_GetDeviceInfo(device);
    PaStreamParameters input_params;
    input_params.device = device;
    input_params.channelCount = APP_MIC_CHANNELS;
    input_params.sampleFormat = paFloat32;
    input_params.suggestedLatency = device_info != 0 ? device_info->defaultLowInputLatency : 0;
    input_params.hostApiSpecificStreamInfo = 0;

    PaStream *stream = 0;
    err = Pa_OpenStream(&stream,
                        &input_params,
                        0,
                        APP_MIC_SAMPLE_RATE,
                        APP_MIC_FRAMES_PER_BUFFER,
                        paClipOff,
                        0,
                        0);
    if (err != paNoError) {
        Pa_Terminate();
        return (int)err;
    }

    params->microphone_stream = stream;
#endif

    return 0;
}

void hardware_params_uninit(HardwareParams *params)
{
    if (params == 0) {
        return;
    }

#if defined(CMSIS_STREAM_NODES_USE_PORTAUDIO)
    if (params->microphone_stream != 0) {
        PaStream *stream = (PaStream *)params->microphone_stream;
        if (Pa_IsStreamActive(stream) == 1) {
            Pa_StopStream(stream);
        }
        Pa_CloseStream(stream);
        params->microphone_stream = 0;
    }
    Pa_Terminate();
#endif
}

int hardware_microphone_start(const HardwareParams *params)
{
    if (params == 0 || params->microphone_stream == 0) {
        return -1;
    }

#if defined(CMSIS_STREAM_NODES_USE_PORTAUDIO)
    PaStream *stream = (PaStream *)params->microphone_stream;
    if (Pa_IsStreamActive(stream) == 1) {
        return 0;
    }

    PaError err = Pa_StartStream(stream);
    return err == paNoError ? 0 : (int)err;
#else
    return -1;
#endif
}

int hardware_microphone_pause(const HardwareParams *params)
{
    if (params == 0 || params->microphone_stream == 0) {
        return -1;
    }

#if defined(CMSIS_STREAM_NODES_USE_PORTAUDIO)
    PaStream *stream = (PaStream *)params->microphone_stream;
    if (Pa_IsStreamActive(stream) != 1) {
        return 0;
    }

    PaError err = Pa_StopStream(stream);
    return err == paNoError ? 0 : (int)err;
#else
    return -1;
#endif
}

int hardware_microphone_resume(const HardwareParams *params)
{
    return hardware_microphone_start(params);
}

int hardware_microphone_read(const HardwareParams *params, float *samples, int32_t frames)
{
    if (params == 0 || samples == 0 || frames <= 0 || params->microphone_stream == 0) {
        return -1;
    }

#if defined(CMSIS_STREAM_NODES_USE_PORTAUDIO)
    PaError err = Pa_ReadStream((PaStream *)params->microphone_stream, samples, frames);
    if (err != paNoError) {
        return (int)err;
    }
    return frames;
#else
    return -1;
#endif
}
