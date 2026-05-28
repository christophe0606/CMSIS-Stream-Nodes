#pragma once

#include "GenericNodes.hpp"
#include "app_params.h"
#include "cg_enums.h"

#include <algorithm>

namespace recorder {

template <typename OUT, int outputSamples>
class DebugSource final : public arm_cmsis_stream::GenericSource<OUT, outputSamples> {
  public:
    explicit DebugSource(arm_cmsis_stream::FIFOBase<OUT> &dst)
        : arm_cmsis_stream::GenericSource<OUT, outputSamples>(dst)
    {
    }

    DebugSource(arm_cmsis_stream::FIFOBase<OUT> &dst, const DebugSourceParams &params)
        : arm_cmsis_stream::GenericSource<OUT, outputSamples>(dst),
          hw_(params.hw_),
          value_(static_cast<OUT>(params.value))
    {
    }

    int run() final
    {
        OUT *output = this->getWriteBuffer();
        std::fill(output, output + outputSamples, value_);
        return CG_SUCCESS;
    }

  private:
    HardwareParams hw_{};
    OUT value_{};
};

} // namespace recorder

using recorder::DebugSource;
