#pragma once

#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "app_params.h"
#include "cg_enums.h"
#include "dsp/basic_math_functions.h"
#include "dsp/statistics_functions.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <vector>

using namespace arm_cmsis_stream;

extern "C" void node_softmax(float *in, size_t blockSize);

class DebugSink final : public StreamNode {
  public:
    explicit DebugSink() : StreamNode()
    {
    }

    cg_status processEvent(int dstPort, Event &&evt) final
    {
        (void)dstPort;
        if (evt.event_id != kValue || inferenceCount_ >= TEST_DURATION_FRAMES) {
            return CG_SUCCESS;
        }

        if (evt.wellFormed<TensorPtr<float>>()) {
            evt.apply<TensorPtr<float>>(&DebugSink::dispTensor, *this);
        } else if (evt.wellFormed<TensorPtr<const float>>()) {
            evt.apply<TensorPtr<const float>>(
                &DebugSink::dispConstTensor, *this);
        } else {
            CMSISSTREAM_LOG_ERR("DebugSink: invalid inference event type\n");
            return CG_OS_ERROR;
        }

        return CG_SUCCESS;
    }

  private:
    void dispConstTensor(const TensorPtr<const float> &tensorPtr)
    {
        bool lockError = false;
        tensorPtr.lock_shared(lockError, [this](const Tensor<const float> &tensor) {
            ++inferenceCount_;
           

            const float *t = tensor.buffer();
            printf("%02d: ", inferenceCount_);
            for (size_t i = 0; i < tensor.size(); ++i) {
               printf("%+1.3f ", t[i]);
            }
            printf("\n");

        });

        if (lockError) {
            CMSISSTREAM_LOG_ERR("DebugSink: inference tensor lock error\n");
        }
    }

    void dispTensor(const TensorPtr<float> &tensorPtr)
    {
        bool lockError = false;
        tensorPtr.lock_shared(lockError, [this](const Tensor<float> &tensor) {
            ++inferenceCount_;
           
            const float *t = tensor.buffer();
            printf("%02d: ", inferenceCount_);
            for (size_t i = 0; i < tensor.size(); ++i) {
               printf("%+1.3f ", t[i]);
            }
            printf("\n");

        });

        if (lockError) {
            CMSISSTREAM_LOG_ERR("DebugSink: inference tensor lock error\n");
        }
    }

    uint32_t inferenceCount_ = 0;
};
