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
static uint8_t schedule[8]=
{ 
4,6,2,0,7,5,1,3,
};

/*

Internal ID identification for the nodes

*/
#define AUDIOWIN_INTERNAL_ID 0
#define FFT_INTERNAL_ID 1
#define GAIN_INTERNAL_ID 2
#define SPECTROGRAM_INTERNAL_ID 3
#define SRC_INTERNAL_ID 4
#define TOCOMPLEX_INTERNAL_ID 5
#define TO_F32_INTERNAL_ID 6
#define WINLEFT_INTERNAL_ID 7
#define DISPLAY_INTERNAL_ID 8



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
#define FIFOSIZE4 1024
#define FIFOSIZE5 1024
#define FIFOSIZE6 1024

#define BUFFERSIZE0 8192
CG_BEFORE_BUFFER
uint8_t stream_app_buf0[BUFFERSIZE0]={0};

#define BUFFERSIZE1 8192
CG_BEFORE_BUFFER
uint8_t stream_app_buf1[BUFFERSIZE1]={0};


typedef struct {
FIFO<q15_t,FIFOSIZE0,1,0> *fifo0;
FIFO<float,FIFOSIZE1,1,0> *fifo1;
FIFO<float,FIFOSIZE2,1,0> *fifo2;
FIFO<float,FIFOSIZE3,1,0> *fifo3;
FIFO<float,FIFOSIZE4,1,0> *fifo4;
FIFO<cf32,FIFOSIZE5,1,0> *fifo5;
FIFO<cf32,FIFOSIZE6,1,0> *fifo6;
} fifos_t;

typedef struct {
    SlidingBuffer<float,640,320> *audioWin;
    CFFT<cf32,1024,cf32,1024> *fft;
    Gain<float,320,float,320> *gain;
    Spectrogram<cf32,1024> *spectrogram;
    MicrophoneSource<q15_t,320> *src;
    RealToComplex<float,1024,cf32,1024> *toComplex;
    Convert<q15_t,320,float,320> *to_f32;
    Hanning<float,640,float,1024> *winLeft;
    SpectrogramTextDisplay *display;
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
    fifos.fifo5 = new (std::nothrow) FIFO<cf32,FIFOSIZE5,1,0>(stream_app_buf0);
    if (fifos.fifo5==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    fifos.fifo6 = new (std::nothrow) FIFO<cf32,FIFOSIZE6,1,0>(stream_app_buf1);
    if (fifos.fifo6==NULL)
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

    nodes.fft = new (std::nothrow) CFFT<cf32,1024,cf32,1024>(*(fifos.fifo5),*(fifos.fifo6));
    if (nodes.fft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.gain = new (std::nothrow) Gain<float,320,float,320>(*(fifos.fifo1),*(fifos.fifo2),params->gain);
    if (nodes.gain==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.spectrogram = new (std::nothrow) Spectrogram<cf32,1024>(*(fifos.fifo6),evtQueue);
    if (nodes.spectrogram==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.src = new (std::nothrow) MicrophoneSource<q15_t,320>(*(fifos.fifo0),params->src);
    if (nodes.src==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APP_SRC_ID]=createStreamNode(*nodes.src);
    nodes.src->setID(STREAM_APP_SRC_ID);

    nodes.toComplex = new (std::nothrow) RealToComplex<float,1024,cf32,1024>(*(fifos.fifo4),*(fifos.fifo5));
    if (nodes.toComplex==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.to_f32 = new (std::nothrow) Convert<q15_t,320,float,320>(*(fifos.fifo0),*(fifos.fifo1));
    if (nodes.to_f32==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APP_TO_F32_ID]=createStreamNode(*nodes.to_f32);
    nodes.to_f32->setID(STREAM_APP_TO_F32_ID);

    nodes.winLeft = new (std::nothrow) Hanning<float,640,float,1024>(*(fifos.fifo3),*(fifos.fifo4));
    if (nodes.winLeft==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }

    nodes.display = new (std::nothrow) SpectrogramTextDisplay;
    if (nodes.display==NULL)
    {
        return(CG_MEMORY_ALLOCATION_FAILURE);
    }
    identifiedNodes[STREAM_APP_DISPLAY_ID]=createStreamNode(*nodes.display);
    nodes.display->setID(STREAM_APP_DISPLAY_ID);


/* Subscribe nodes for the event system*/
    nodes.spectrogram->subscribe(0,*nodes.display,0);

    initError = CG_SUCCESS;
    initError = nodes.audioWin->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.fft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.gain->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.spectrogram->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.src->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.toComplex->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.to_f32->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.winLeft->init();
    if (initError != CG_SUCCESS)
        return(initError);
    
    initError = nodes.display->init();
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
    if (fifos.fifo6!=NULL)
    {
       delete fifos.fifo6;
    }

    if (nodes.audioWin!=NULL)
    {
        delete nodes.audioWin;
    }
    if (nodes.fft!=NULL)
    {
        delete nodes.fft;
    }
    if (nodes.gain!=NULL)
    {
        delete nodes.gain;
    }
    if (nodes.spectrogram!=NULL)
    {
        delete nodes.spectrogram;
    }
    if (nodes.src!=NULL)
    {
        delete nodes.src;
    }
    if (nodes.toComplex!=NULL)
    {
        delete nodes.toComplex;
    }
    if (nodes.to_f32!=NULL)
    {
        delete nodes.to_f32;
    }
    if (nodes.winLeft!=NULL)
    {
        delete nodes.winLeft;
    }
    if (nodes.display!=NULL)
    {
        delete nodes.display;
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
    if (fifos.fifo6!=NULL)
    {
       fifos.fifo6->reset();
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
        for(; id < 8; id++)
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
                    
                   cgStaticError = nodes.fft->run();
                }
                break;

                case 2:
                {
                    
                   cgStaticError = nodes.gain->run();
                }
                break;

                case 3:
                {
                    
                   cgStaticError = nodes.spectrogram->run();
                }
                break;

                case 4:
                {
                    
                   cgStaticError = nodes.src->run();
                }
                break;

                case 5:
                {
                    
                   cgStaticError = nodes.toComplex->run();
                }
                break;

                case 6:
                {
                    
                   cgStaticError = nodes.to_f32->run();
                }
                break;

                case 7:
                {
                    
                   cgStaticError = nodes.winLeft->run();
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
