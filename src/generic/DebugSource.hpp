#pragma once

#include "GenericNodes.hpp"
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

    int run() final
    {
        OUT *output = this->getWriteBuffer();
        std::fill(output, output + outputSamples, OUT{});
        return CG_SUCCESS;
    }
};

} // namespace recorder

using recorder::DebugSource;
