#pragma once



#include "cg_enums.h"
#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"


using namespace arm_cmsis_stream;


template <typename OUT, int outputSamples>
class NullSink: public GenericSink<OUT, outputSamples>
{
  public:
   
    NullSink(FIFOBase<OUT> &dst,EventQueue *queue)
        : GenericSink<OUT, outputSamples>(dst),ev(queue)
    {

       
    };

    ~NullSink()
    {
        
    };


    int run() final
    {
        OUT *input = this->getReadBuffer();
        (void)input; // Suppress unused variable warning

        ev.sendAsync(cg_event_priority::kNormalPriority, kDo);
        
        return (CG_SUCCESS);
    };

    void processValue(uint32_t v)
    {
    }

    cg_status processEvent(int dstPort, Event &&evt) final override
    {
        if (dstPort == 0)
        {
            if (evt.event_id == kValue)
            {
                if (evt.wellFormed<uint32_t>())
                {
                    evt.apply<uint32_t>(&NullSink::processValue, *this);
                }
            }
        }
        return CG_SUCCESS;
    };

    void subscribe(int outputPort, StreamNode &dst, int dstPort) final override
    {
        if (outputPort == 0)
           ev.subscribe(dst, dstPort);
    }

protected:
   EventOutput ev;
  
};