#pragma once

#include "cg_enums.h"
#include "stream_rtos_events.h"
#include "stream_runtime_init.hpp"

extern "C" {
#include "datatypes.h"
#include "app_params.h"
}

#define CG_BEFORE_BUFFER alignas(16)

#define CG_BEFORE_NODE_EXECUTION(id)                                                \
    {                                                                               \
        uint32_t res = cg_streamEvent.wait(STREAM_PAUSE_EVENT | STREAM_DONE_EVENT,  \
                                           true, 0);                                \
        if ((res & STREAM_DONE_EVENT) != 0U)                                        \
        {                                                                           \
            cgStaticError = CG_STOP_SCHEDULER;                                      \
            goto errorHandling;                                                     \
        }                                                                           \
        if ((res & STREAM_PAUSE_EVENT) != 0U)                                       \
        {                                                                           \
            cgStaticError = CG_PAUSED_SCHEDULER;                                    \
            goto errorHandling;                                                     \
        }                                                                           \
    }
