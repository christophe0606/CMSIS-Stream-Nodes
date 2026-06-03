#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void *microphone_stream;
    int32_t microphone_sample_rate;
    int32_t microphone_num_channels;
} HardwareParams;

int hardware_params_init(HardwareParams *params);
void hardware_params_uninit(HardwareParams *params);
int hardware_microphone_start(const HardwareParams *params);
int hardware_microphone_read(const HardwareParams *params, float *samples, int32_t frames);

#ifdef __cplusplus
}
#endif
