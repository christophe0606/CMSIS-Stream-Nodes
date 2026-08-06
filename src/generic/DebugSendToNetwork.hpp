#pragma once

#include "EventQueue.hpp"
#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "cg_enums.h"

#include <cstring>

using namespace arm_cmsis_stream;

template <typename IN, int inputSamples>
class DebugSendToNetwork final : public GenericSink<IN, inputSamples> {
  public:
    DebugSendToNetwork(FIFOBase<IN> &src, EventQueue *queue)
        : GenericSink<IN, inputSamples>(src), output_(queue)
    {
    }

    int run() final
    {
        const IN *input = this->getReadBuffer();
        UniquePtr<IN> tensorData(inputSamples);
        std::memcpy(tensorData.get(), input, inputSamples * sizeof(IN));

        TensorPtr<IN> tensor = TensorPtr<IN>::create_with(
            static_cast<uint8_t>(1),
            cg_tensor_dims_t{inputSamples},
            std::move(tensorData));
        output_.sendSync(kNormalPriority, kValue, std::move(tensor));
        return CG_SUCCESS;
    }

    void subscribe(int outputPort, StreamNode &dst, int dstPort) final
    {
        if (outputPort == 0) {
            output_.subscribe(dst, dstPort);
        }
    }

  private:
    EventOutput output_;
};
