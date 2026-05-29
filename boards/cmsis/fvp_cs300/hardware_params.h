#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    float sampling_rate;
    uint8_t *audio_samples;
    size_t buffer_size;
} HardwareParams;

int hardware_params_init(HardwareParams *params);
void hardware_params_uninit(HardwareParams *params);
