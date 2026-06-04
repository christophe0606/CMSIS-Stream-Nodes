#ifndef HARDWARE_PARAMS_H
#define HARDWARE_PARAMS_H

#include <stdint.h>

typedef int32_t HardwareParams;

int hardware_params_init(HardwareParams *params);
void hardware_params_uninit(HardwareParams *params);

#endif /* HARDWARE_PARAMS_H */
