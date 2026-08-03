#pragma once


#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
#include "cg_enums.h"
#include <cstring>
#include <atomic>

using namespace arm_cmsis_stream;

template <typename IN, int inputSamples>
class SendToNetwork : public GenericSink<IN, inputSamples>, public ContextSwitch
{
  public:
    // Array used to map local selector IDs to global selector ID
    // Global IDs are graph dependent and may change when the node is used in different graphs.
    // Here there is only one ID for the "ack" event defined in the Python
    enum selector {selAck=0};
    static std::array<uint16_t,1> selectors;

    SendToNetwork(FIFOBase<IN> &src, EventQueue *queue)
        : GenericSink<IN, inputSamples>(src), ev0(queue) {
          };

   
    int run() override final
    {
        IN *in = this->getReadBuffer();
        if (ready.load())
        {

            UniquePtr<IN> tensorData(inputSamples);
            memcpy(tensorData.get(), in, inputSamples * sizeof(IN));

            TensorPtr<IN> t = TensorPtr<IN>::create_with((uint8_t)1,
                                                         cg_tensor_dims_t{inputSamples},
                                                         std::move(tensorData));
            bool status = ev0.sendAsync(kHighPriority, kValue, std::move(t)); // Send the event to the subscribed nodes

            if (!status)
            {
                //CMSISSTREAM_LOG_ERR("SendToNetwork: Failed to send event to network\n");
            }
            else 
            {
                ready.store(false);
            }
        }

        return (CG_SUCCESS);
    };

    int pause() final override
	{
        // So that a new frame can be sent after resume
        ready.store(true);
		return 0;
	}

	int resume() final override
	{

		return 0;
	}

    cg_status processEvent(int dstPort, Event &&evt) final override
    {
        if (dstPort == 0)
        {
            // If "ack" event was received
            if (evt.event_id == selectors[selAck])
            {
                ready.store(true);
            }
        }
        return CG_SUCCESS;
    }

    void subscribe(int outputPort, StreamNode &dst, int dstPort) final override
    {
        ev0.subscribe(dst, dstPort);
    }

  protected:
    std::atomic<bool> ready{false};
    EventOutput ev0;
};