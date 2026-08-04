#ifndef HARDWARE_PARAMS_H
#define HARDWARE_PARAMS_H

#include <stdint.h>
#include <stddef.h>

struct device;
struct k_mem_slab;

typedef struct {
    const struct device *microphone_device;
    struct k_mem_slab *microphone_mem_slab;
    int32_t microphone_sample_rate;
    int32_t microphone_num_channels;
    const uint8_t *model_weights;
    size_t model_size;
} HardwareParams;

int hardware_params_init(HardwareParams *params);
void hardware_params_uninit(HardwareParams *params);

#endif /* HARDWARE_PARAMS_H */
