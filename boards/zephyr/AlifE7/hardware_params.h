#ifndef HARDWARE_PARAMS_H
#define HARDWARE_PARAMS_H

#include <stdint.h>

struct device;
struct k_mem_slab;

typedef struct {
    const struct device *microphone_device;
    struct k_mem_slab *microphone_mem_slab;
    int32_t microphone_sample_rate;
    int32_t microphone_num_channels;
} HardwareParams;

int hardware_params_init(HardwareParams *params);
void hardware_params_uninit(HardwareParams *params);

#endif /* HARDWARE_PARAMS_H */
