/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/

#ifndef SCHEDULER_RECORDER_H_ 
#define SCHEDULER_RECORDER_H_


#include <stdint.h>

#ifdef   __cplusplus
extern "C"
{
#endif

#include "cstream_node.h"


/* Node identifiers */
#define STREAM_RECORDER_NB_IDENTIFIED_NODES 2
#define STREAM_RECORDER_SINK_ID 0
#define STREAM_RECORDER_SRC_ID 1

#define STREAM_RECORDER_SCHED_LEN 2


extern CStreamNode* get_scheduler_recorder_node(int32_t nodeID);

extern int init_scheduler_recorder(void *evtQueue_);
extern void free_scheduler_recorder();
extern uint32_t scheduler_recorder(int *error);
extern void reset_fifos_scheduler_recorder(int all);

#ifdef   __cplusplus
}
#endif

#endif

