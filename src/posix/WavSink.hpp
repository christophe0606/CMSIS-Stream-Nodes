#pragma once

#include "GenericNodes.hpp"
#include "cg_enums.h"
#include "posix/WavSource.hpp"
#include "tinywav/tinywav.h"

#include <array>
#include <cstdint>
#include <type_traits>

namespace cmsis_stream_nodes {

struct WavSinkParams {
    const char *path;
    int32_t sample_rate;
    int32_t num_channels;
    int32_t sample_format;
    int32_t channel_format;
};

namespace detail {

inline TinyWavSampleFormat normalizeWavSampleFormat(int32_t format)
{
    switch (format) {
    case TW_FLOAT32:
        return TW_FLOAT32;
    case TW_INT16:
    default:
        return TW_INT16;
    }
}

} // namespace detail

template <typename IN, int inputSamples>
class WavSink final : public arm_cmsis_stream::GenericSink<IN, inputSamples> {
    static_assert(detail::isSupportedWavType<IN>(), "WavSink supports float, q15_t, sf32, and sq15 samples");

  public:
    template <typename Params>
    WavSink(arm_cmsis_stream::FIFOBase<IN> &src, const Params &params)
        : arm_cmsis_stream::GenericSink<IN, inputSamples>(src),
          numChannels_(params.num_channels)
    {
        const char *path = params.path != nullptr ? params.path : "";
        opened_ = tinywav_open_write(
                      &wav_,
                      static_cast<int16_t>(params.num_channels),
                      params.sample_rate,
                      detail::normalizeWavSampleFormat(params.sample_format),
                      detail::normalizeWavChannelFormat(params.channel_format),
                      path) == 0;
    }

    ~WavSink()
    {
        if (opened_ && tinywav_isOpen(&wav_)) {
            tinywav_close_write(&wav_);
        }
    }

    int run() final
    {
        if (!opened_ || !tinywav_isOpen(&wav_) || numChannels_ <= 0 || numChannels_ > 2) {
            return CG_BUFFER_ERROR;
        }

        const IN *input = this->getReadBuffer();
        const int framesToWrite = inputSamples;
        const int samplesToWrite = framesToWrite * numChannels_;
        detail::copyToWavFrames(frameBuffer_.data(), input, samplesToWrite);
        const int framesWritten = tinywav_write_f(&wav_, frameBuffer_.data(), framesToWrite);

        return framesWritten == framesToWrite ? CG_SUCCESS : CG_BUFFER_OVERFLOW;
    }

  private:
    TinyWav wav_{};
    int32_t numChannels_;
    std::array<float, inputSamples * 2> frameBuffer_{};
    bool opened_ = false;
};

} // namespace cmsis_stream_nodes

using cmsis_stream_nodes::WavSink;
using cmsis_stream_nodes::WavSinkParams;
