#pragma once

#include "debug_audio.h"
#include "GenericNodes.hpp"
#include "cg_enums.h"

#include <algorithm>
#include <cstddef>

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
        const std::size_t remaining = DEBUG_AUDIO_SAMPLE_COUNT - sampleOffset_;
        const std::size_t copied = std::min<std::size_t>(outputSamples, remaining);

        std::copy_n(debug_audio_samples + sampleOffset_, copied, output);
        std::fill(output + copied, output + outputSamples, static_cast<OUT>(0));
        sampleOffset_ += copied;
        return CG_SUCCESS;
    }

  private:
    std::size_t sampleOffset_ = 0;
};

} // namespace recorder

using recorder::DebugSource;
