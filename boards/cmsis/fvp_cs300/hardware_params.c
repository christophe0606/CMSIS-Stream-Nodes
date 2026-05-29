#include "hardware_params.h"

int hardware_params_init(HardwareParams *params)
{
    if (params == 0) {
        return -1;
    }
    params->sampling_rate = 44100.0f;
    params->audio_samples = NULL;
    params->buffer_size = 0;
    return 0;
}

void hardware_params_uninit(HardwareParams *params)
{
    (void)params;
}
