#pragma once

#include <stdint.h>

typedef int32_t HardwareParams;

int hardware_params_init(HardwareParams *params);
void hardware_params_uninit(HardwareParams *params);
