#ifndef HARDWARE_PARAMS_H
#define HARDWARE_PARAMS_H

#include <stdint.h>

#define HARDWARE_MICROPHONE_FRAME_EVENT (1U << 0)
#define HARDWARE_MICROPHONE_OVERFLOW_EVENT (1U << 1)

typedef struct {
    void *microphone_stream;
    void *microphone_event;
    int32_t microphone_sample_rate;
    int32_t microphone_num_channels;
} HardwareParams;

int hardware_params_init(HardwareParams *params);
void hardware_params_uninit(HardwareParams *params);
int hardware_audio_pause(HardwareParams *params);
int hardware_audio_resume(HardwareParams *params);

#endif /* HARDWARE_PARAMS_H */
