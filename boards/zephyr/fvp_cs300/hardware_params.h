#pragma once

#include <stdint.h>

typedef struct {
    int32_t dummy;
} HardwareParams;

int hardware_params_init(HardwareParams *params);
void hardware_params_uninit(HardwareParams *params);
