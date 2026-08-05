/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/

#ifndef SCHEDULER_APP_H_ 
#define SCHEDULER_APP_H_


#include <stdint.h>

#ifdef   __cplusplus
extern "C"
{
#endif

#include "cstream_node.h"


/* Node identifiers */
#define STREAM_APP_NB_IDENTIFIED_NODES 8
#define STREAM_APP_AUDIOWIN_ID 0
#define STREAM_APP_MFCC_ID 1
#define STREAM_APP_MFCCWIN_ID 2
#define STREAM_APP_SEND_ID 3
#define STREAM_APP_SRC_ID 4
#define STREAM_APP_TO_F32_ID 5
#define STREAM_APP_CLASSIFY_ID 6
#define STREAM_APP_KWS_ID 7

#define STREAM_APP_SCHED_LEN 8

/* Selectors global identifiers */
#define SEL_ACK_ID 100 

extern CStreamNode* get_scheduler_app_node(int32_t nodeID);

extern int init_scheduler_app(void *evtQueue_,AppParams *params);
extern void free_scheduler_app();
extern uint32_t scheduler_app(int *error);
extern void reset_fifos_scheduler_app(int all);

#ifdef   __cplusplus
}
#endif

#endif

