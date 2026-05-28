#pragma once

#include "GenericNodes.hpp"
#include "cg_enums.h"

#include <algorithm>
#include <cstddef>
#include <type_traits>

namespace cmsis_stream_nodes {

template <typename T>
struct BufferSourceParams {
    const T *buffer;
    std::size_t length;
};

template <typename OUT, int outputSamples>
class BufferSource final : public arm_cmsis_stream::GenericSource<OUT, outputSamples> {
    static_assert(std::is_arithmetic<OUT>::value, "BufferSource only supports numeric datatypes");

  public:
    template <typename Params>
    BufferSource(arm_cmsis_stream::FIFOBase<OUT> &dst, const Params &params)
        : arm_cmsis_stream::GenericSource<OUT, outputSamples>(dst),
          buffer_(params.buffer),
          length_(params.length)
    {
    }

    int run() final
    {
        if (buffer_ == nullptr) {
            return CG_BUFFER_ERROR;
        }
        if (position_ >= length_) {
            return CG_STOP_SCHEDULER;
        }

        OUT *output = this->getWriteBuffer();
        const std::size_t available = length_ - position_;
        const std::size_t copied = std::min<std::size_t>(available, outputSamples);

        std::copy_n(buffer_ + position_, copied, output);
        std::fill(output + copied, output + outputSamples, OUT{});

        position_ += copied;
        return CG_SUCCESS;
    }

  private:
    const OUT *buffer_;
    std::size_t length_;
    std::size_t position_ = 0;
};

} // namespace cmsis_stream_nodes

using cmsis_stream_nodes::BufferSource;
using cmsis_stream_nodes::BufferSourceParams;
