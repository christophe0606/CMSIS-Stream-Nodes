#pragma once

#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "app_params.h"
#include "cg_enums.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <vector>

#include "common/KWSClassifyCompute.hpp"

using namespace arm_cmsis_stream;


class DebugKWSClassify final : public StreamNode {
  public:
    explicit DebugKWSClassify(EventQueue *queue) : StreamNode(), output_(queue), compute_(10)
    {
    }

    cg_status processEvent(int dstPort, Event &&evt) final
    {
        (void)dstPort;
        if (evt.event_id != kValue) {
            return CG_SUCCESS;
        }

        if (evt.wellFormed<TensorPtr<float>>()) {
            evt.apply<TensorPtr<float>>(&DebugKWSClassify::sendTensor, *this);
        } else if (evt.wellFormed<TensorPtr<const float>>()) {
            evt.apply<TensorPtr<const float>>(
                &DebugKWSClassify::sendConstTensor, *this);
        } else {
            CMSISSTREAM_LOG_ERR("DebugKWSClassify: invalid inference event type\n");
            return CG_OS_ERROR;
        }

        return CG_SUCCESS;
    }

    void sendTensor(TensorPtr<float> &&tensorPtr)
    {

        bool lockError = false;
        tensorPtr.lock_shared(lockError, [this](const Tensor<float> &tensor) {
           uint32_t nb = tensor.size();
           (void)compute_(tensor.buffer());
           UniquePtr<const float> tensorData(compute_.latest());
           TensorPtr<const float> t = TensorPtr<const float>::create_with((uint8_t)1,
                                                         cg_tensor_dims_t{nb},
                                                         std::move(tensorData));

           output_.sendSync(kNormalPriority, kValue, std::move(t));
        });
    }

    void sendConstTensor(TensorPtr<const float> &&tensorPtr)
    {
        bool lockError = false;
        tensorPtr.lock_shared(lockError, [this](const Tensor<const float> &tensor) {
           uint32_t nb = tensor.size();
           (void)compute_(tensor.buffer());
           UniquePtr<const float> tensorData(compute_.latest());
           TensorPtr<const float> t = TensorPtr<const float>::create_with((uint8_t)1,
                                                         cg_tensor_dims_t{nb},
                                                         std::move(tensorData));

           output_.sendSync(kNormalPriority, kValue, std::move(t));
        });
    }

    void subscribe(int outputPort, StreamNode &dst, int dstPort) final
    {
        if (outputPort == 0) 
        {
            output_.subscribe(dst, dstPort);
        }
    }

private:
    EventOutput output_;
    KWSClassifyCompute compute_;
};
