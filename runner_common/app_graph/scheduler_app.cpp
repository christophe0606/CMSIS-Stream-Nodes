/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/


#include <cstdint>
#include "app_config.hpp"
#include "stream_platform_config.hpp"
#include "cg_enums.h"
#include "StreamNode.hpp"
#include "cstream_node.h"
#include "IdentifiedNode.hpp"
#include "EventQueue.hpp"
#include "GenericNodes.hpp"
#include "AppNodes.hpp"
#include "scheduler_app.h"

#if !defined(CHECKERROR)
#define CHECKERROR       if (cgStaticError < 0) \
       {\
         goto errorHandling;\
       }

#endif


#if !defined(CG_BEFORE_ITERATION)
#define CG_BEFORE_ITERATION
#endif 

#if !defined(CG_AFTER_ITERATION)
#define CG_AFTER_ITERATION
#endif 

#if !defined(CG_BEFORE_SCHEDULE)
#define CG_BEFORE_SCHEDULE
#endif

#if !defined(CG_AFTER_SCHEDULE)
#define CG_AFTER_SCHEDULE
#endif

#if !defined(CG_BEFORE_BUFFER)
#define CG_BEFORE_BUFFER
#endif

#if !defined(CG_BEFORE_FIFO_BUFFERS)
#define CG_BEFORE_FIFO_BUFFERS
#endif

#if !defined(CG_BEFORE_FIFO_INIT)
#define CG_BEFORE_FIFO_INIT
#endif

#if !defined(CG_BEFORE_NODE_INIT)
#define CG_BEFORE_NODE_INIT
#endif

#if !defined(CG_AFTER_INCLUDES)
#define CG_AFTER_INCLUDES
#endif

#if !defined(CG_BEFORE_SCHEDULER_FUNCTION)
#define CG_BEFORE_SCHEDULER_FUNCTION
#endif

#if !defined(CG_BEFORE_NODE_EXECUTION)
#define CG_BEFORE_NODE_EXECUTION(ID)
#endif

#if !defined(CG_AFTER_NODE_EXECUTION)
#define CG_AFTER_NODE_EXECUTION(ID)
#endif





CG_AFTER_INCLUDES


using namespace arm_cmsis_stream;

/*

Description of the scheduling. 

*/
static uint8_t schedule[7]=
{ 
5,6,1,0,2,3,4,
};

/*

Internal ID identification for the nodes

*/
#define AUDIOWIN_INTERNAL_ID 0
#define GAIN_INTERNAL_ID 1
#define MFCC_INTERNAL_ID 2
#define MFCCWIN_INTERNAL_ID 3
#define SEND_INTERNAL_ID 4
#define SRC_INTERNAL_ID 5
#define TO_F32_INTERNAL_ID 6
#define CLASSIFY_INTERNAL_ID 7
#define KWS_INTERNAL_ID 8

/* Initialize the selectors global IDs in each class */
template<>
std::array<uint16_t,1> SendToNetwork<float,490>::selectors = {SEL_ACK_ID};
std::array<uint16_t,1> KWS::selectors = {SEL_ACK_ID};


/***********

Node identification

************/
static CStreamNode identifiedNodes[STREAM_APP_NB_IDENTIFIED_NODES]={0};

CG_BEFORE_FIFO_BUFFERS
/***********

FIFO buffers

************/
#define FIFOSIZE0 320
#define FIFOSIZE1 320
#define FIFOSIZE2 320
#define FIFOSIZE3 640
#define FIFOSIZE4 10
#define FIFOSIZE5 490

#define BUFFERSIZE0 2560
CG_BEFORE_BUFFER
uint8_t stream_app_buf0[BUFFERSIZE0]={0};

#define BUFFERSIZE1 1280
CG_BEFORE_BUFFER
uint8_t stream_app_buf1[BUFFERSIZE1]={0};


typedef struct {
FIFO<q15_t,FIFOSIZE0,1,0> *fifo0;
FIFO<float,FIFOSIZE1,1,0> *fifo1;
FIFO<float,FIFOSIZE2,1,0> *fifo2;
FIFO<float,FIFOSIZE3,1,0> *fifo3;
FIFO<float,FIFOSIZE4,1,0> *fifo4;
FIFO<float,FIFOSIZE5,1,0> *fifo5;
} fifos_t;

typedef struct {
    SlidingBuffer<float,640,320> *audioWin;
    Gain<float,320,float,320> *gain;
    MFCC<float,640,float,10> *mfcc;
    SlidingBuffer<float,490,480> *mfccWin;
    SendToNetwork<float,490> *send;
    MicrophoneSource<q15_t,320> *src;
    Convert<q15_t,320,float,320> *to_f32;
    KWSClassify *classify;
    KWS *kws;
} nodes_t;


static fifos_t fifos={0};

static nodes_t nodes={0};

CStreamNode* get_scheduler_app_node(int32_t nodeID)
{
    if (nodeID >= STREAM_APP_NB_IDENTIFIED_NODES)
    {
        return(nullptr);
    }
    if (nodeID < 0)
    {
        return(nullptr);
    }
    return(&identifiedNodes[nodeID]);
}

int init_scheduler_app(void *evtQueue_,AppParams *params)
{
    EventQueue *evtQueue = reinterpret_cast<EventQueue *>(evtQueue_);
    (void)evtQueue;

    CG_BEFORE_FIFO_INIT;
    fifos.fifo0 = new (std::nothrow) FIFO<q15_t,FIFOSIZE0,1,0>(stream_app_buf1);
    if (fifos.fifo0==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo1 = new (std::nothrow) FIFO<float,FIFOSIZE1,1,0>(stream_app_buf0);
    if (fifos.fifo1==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo2 = new (std::nothrow) FIFO<float,FIFOSIZE2,1,0>(stream_app_buf1);
    if (fifos.fifo2==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo3 = new (std::nothrow) FIFO<float,FIFOSIZE3,1,0>(stream_app_buf0);
    if (fifos.fifo3==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo4 = new (std::nothrow) FIFO<float,FIFOSIZE4,1,0>(stream_app_buf1);
    if (fifos.fifo4==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo5 = new (std::nothrow) FIFO<float,FIFOSIZE5,1,0>(stream_app_buf0);
    if (fifos.fifo5==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    CG_BEFORE_NODE_INIT;
    cg_status initError;

    nodes.audioWin = new (std::nothrow) SlidingBuffer<float,640,320>(*(fifos.fifo2),*(fifos.fifo3));
    if (nodes.audioWin==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APP_AUDIOWIN_ID]=createStreamNode(*nodes.audioWin);
    nodes.audioWin->setID(STREAM_APP_AUDIOWIN_ID);

    nodes.gain = new (std::nothrow) Gain<float,320,float,320>(*(fifos.fifo1),*(fifos.fifo2),params->gain);
    if (nodes.gain==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.mfcc = new (std::nothrow) MFCC<float,640,float,10>(*(fifos.fifo3),*(fifos.fifo4));
    if (nodes.mfcc==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APP_MFCC_ID]=createStreamNode(*nodes.mfcc);
    nodes.mfcc->setID(STREAM_APP_MFCC_ID);

    nodes.mfccWin = new (std::nothrow) SlidingBuffer<float,490,480>(*(fifos.fifo4),*(fifos.fifo5));
    if (nodes.mfccWin==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APP_MFCCWIN_ID]=createStreamNode(*nodes.mfccWin);
    nodes.mfccWin->setID(STREAM_APP_MFCCWIN_ID);

    nodes.send = new (std::nothrow) SendToNetwork<float,490>(*(fifos.fifo5),evtQueue);
    if (nodes.send==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APP_SEND_ID]=createStreamNode(*nodes.send);
    nodes.send->setID(STREAM_APP_SEND_ID);

    nodes.src = new (std::nothrow) MicrophoneSource<q15_t,320>(*(fifos.fifo0),params->src);
    if (nodes.src==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APP_SRC_ID]=createStreamNode(*nodes.src);
    nodes.src->setID(STREAM_APP_SRC_ID);

    nodes.to_f32 = new (std::nothrow) Convert<q15_t,320,float,320>(*(fifos.fifo0),*(fifos.fifo1));
    if (nodes.to_f32==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APP_TO_F32_ID]=createStreamNode(*nodes.to_f32);
    nodes.to_f32->setID(STREAM_APP_TO_F32_ID);

    nodes.classify = new (std::nothrow) KWSClassify(evtQueue,params->classify);
    if (nodes.classify==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APP_CLASSIFY_ID]=createStreamNode(*nodes.classify);
    nodes.classify->setID(STREAM_APP_CLASSIFY_ID);

    nodes.kws = new (std::nothrow) KWS(evtQueue,params->kws);
    if (nodes.kws==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APP_KWS_ID]=createStreamNode(*nodes.kws);
    nodes.kws->setID(STREAM_APP_KWS_ID);


/* Subscribe nodes for the event system*/
    nodes.send->subscribe(0,*nodes.kws,0);
    nodes.kws->subscribe(0,*nodes.send,0);
    nodes.kws->subscribe(1,*nodes.classify,0);

    initError = CG_SUCCESS;
    initError = nodes.audioWin->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.gain->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.mfcc->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.mfccWin->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.send->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.src->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.to_f32->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.classify->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.kws->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
   


    return(CG_SUCCESS);

}

void free_scheduler_app()
{
    if (fifos.fifo0!=NULL)
    {
       delete fifos.fifo0;
    }
    if (fifos.fifo1!=NULL)
    {
       delete fifos.fifo1;
    }
    if (fifos.fifo2!=NULL)
    {
       delete fifos.fifo2;
    }
    if (fifos.fifo3!=NULL)
    {
       delete fifos.fifo3;
    }
    if (fifos.fifo4!=NULL)
    {
       delete fifos.fifo4;
    }
    if (fifos.fifo5!=NULL)
    {
       delete fifos.fifo5;
    }

    if (nodes.audioWin!=NULL)
    {
        delete nodes.audioWin;
    }
    if (nodes.gain!=NULL)
    {
        delete nodes.gain;
    }
    if (nodes.mfcc!=NULL)
    {
        delete nodes.mfcc;
    }
    if (nodes.mfccWin!=NULL)
    {
        delete nodes.mfccWin;
    }
    if (nodes.send!=NULL)
    {
        delete nodes.send;
    }
    if (nodes.src!=NULL)
    {
        delete nodes.src;
    }
    if (nodes.to_f32!=NULL)
    {
        delete nodes.to_f32;
    }
    if (nodes.classify!=NULL)
    {
        delete nodes.classify;
    }
    if (nodes.kws!=NULL)
    {
        delete nodes.kws;
    }
}

void reset_fifos_scheduler_app(int all)
{
    if (fifos.fifo0!=NULL)
    {
       fifos.fifo0->reset();
    }
    if (fifos.fifo1!=NULL)
    {
       fifos.fifo1->reset();
    }
    if (fifos.fifo2!=NULL)
    {
       fifos.fifo2->reset();
    }
    if (fifos.fifo3!=NULL)
    {
       fifos.fifo3->reset();
    }
    if (fifos.fifo4!=NULL)
    {
       fifos.fifo4->reset();
    }
    if (fifos.fifo5!=NULL)
    {
       fifos.fifo5->reset();
    }
   // Buffers are set to zero too
   if (all)
   {
       std::fill_n(stream_app_buf0, BUFFERSIZE0, (uint8_t)0);
       std::fill_n(stream_app_buf1, BUFFERSIZE1, (uint8_t)0);
   }
}


CG_BEFORE_SCHEDULER_FUNCTION
uint32_t scheduler_app(int *error)
{
    int cgStaticError=0;
    uint32_t nbSchedule=0;






    /* Run several schedule iterations */
    CG_BEFORE_SCHEDULE;
    while(cgStaticError==0)
    {
        /* Run a schedule iteration */
        CG_BEFORE_ITERATION;
        unsigned long id=0;
        for(; id < 7; id++)
        {
            CG_BEFORE_NODE_EXECUTION(schedule[id]);
            switch(schedule[id])
            {
                case 0:
                {
                    
                   cgStaticError = nodes.audioWin->run();
                }
                break;

                case 1:
                {
                    
                   cgStaticError = nodes.gain->run();
                }
                break;

                case 2:
                {
                    
                   cgStaticError = nodes.mfcc->run();
                }
                break;

                case 3:
                {
                    
                   cgStaticError = nodes.mfccWin->run();
                }
                break;

                case 4:
                {
                    
                   cgStaticError = nodes.send->run();
                }
                break;

                case 5:
                {
                    
                   cgStaticError = nodes.src->run();
                }
                break;

                case 6:
                {
                    
                   cgStaticError = nodes.to_f32->run();
                }
                break;

                default:
                break;
            }
            CG_AFTER_NODE_EXECUTION(schedule[id]);
                        CHECKERROR;
        }
       CG_AFTER_ITERATION;
       nbSchedule++;
    }
errorHandling:
    CG_AFTER_SCHEDULE;
    *error=cgStaticError;
    return(nbSchedule);
    
}
