#pragma once

#include "GenericNodes.hpp"
#include "cg_enums.h"

#include <cstddef>
#include <cstdio>

namespace recorder {

template <typename IN, int inputSamples>
class DebugSink final : public arm_cmsis_stream::GenericSink<IN, inputSamples> {
  public:
    explicit DebugSink(arm_cmsis_stream::FIFOBase<IN> &src)
        : arm_cmsis_stream::GenericSink<IN, inputSamples>(src)
    {
    }

    int run() final
    {
        const IN *input = this->getReadBuffer();
        (void)input;

        totalSamples_ += inputSamples;
        ++runCount_;

        std::printf("DebugSink run %zu: total samples received = %zu\n",
                    runCount_,
                    totalSamples_);
        return CG_SUCCESS;
    }

  private:
    std::size_t totalSamples_ = 0;
    std::size_t runCount_ = 0;
};

} // namespace recorder

using recorder::DebugSink;
