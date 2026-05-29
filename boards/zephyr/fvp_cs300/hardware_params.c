#include "hardware_params.h"

int hardware_params_init(HardwareParams *params)
{
    if (params == 0) {
        return -1;
    }
    *params = (HardwareParams){0};
    return 0;
}

void hardware_params_uninit(HardwareParams *params)
{
    (void)params;
}
