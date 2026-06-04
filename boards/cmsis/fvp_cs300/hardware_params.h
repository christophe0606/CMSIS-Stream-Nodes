#ifndef HARDWARE_PARAMS_H
#define HARDWARE_PARAMS_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    float sampling_rate;
    uint8_t *audio_samples;
    size_t buffer_size;
} HardwareParams;

int hardware_params_init(HardwareParams *params);
void hardware_params_uninit(HardwareParams *params);
int hardware_audio_pause(HardwareParams *params);
int hardware_audio_resume(HardwareParams *params);

#endif /* HARDWARE_PARAMS_H */
