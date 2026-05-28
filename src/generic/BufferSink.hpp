#pragma once

#include "GenericNodes.hpp"
#include "cg_enums.h"

#include <algorithm>
#include <cstddef>
#include <type_traits>

namespace cmsis_stream_nodes {

template <typename T>
struct BufferSinkParams {
    T *buffer;
    std::size_t length;
    bool repeat;
};

template <typename IN, int inputSamples>
class BufferSink final : public arm_cmsis_stream::GenericSink<IN, inputSamples> {
    static_assert(std::is_arithmetic<IN>::value, "BufferSink only supports numeric datatypes");

  public:
    template <typename Params>
    BufferSink(arm_cmsis_stream::FIFOBase<IN> &src, Params &params)
        : arm_cmsis_stream::GenericSink<IN, inputSamples>(src),
          buffer_(params.buffer),
          length_(params.length),
          repeat_(params.repeat)
    {
    }

    int run() final
    {
        if (buffer_ == nullptr) {
            return CG_BUFFER_ERROR;
        }

        const IN *input = this->getReadBuffer();

        for (int i = 0; i < inputSamples; ++i) {
            if (writePosition_ >= length_) {
                if (!repeat_) {
                    break;
                }
                writePosition_ = 0;
            }

            buffer_[writePosition_++] = input[i];
        }

        return CG_SUCCESS;
    }

  private:
    IN *buffer_;
    std::size_t length_;
    bool repeat_;
    std::size_t writePosition_ = 0;
};

} // namespace cmsis_stream_nodes

using cmsis_stream_nodes::BufferSink;
using cmsis_stream_nodes::BufferSinkParams;
