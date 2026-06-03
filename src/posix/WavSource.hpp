#pragma once

#include "GenericNodes.hpp"
#include "cg_enums.h"
#include "datatypes.h"
#include "dsp/support_functions.h"
#include "tinywav/tinywav.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <type_traits>

namespace cmsis_stream_nodes {

struct WavSourceParams {
    const char *path;
    int32_t num_channels;
    int32_t sample_format;
    int32_t channel_format;
};

namespace detail {

inline TinyWavChannelFormat normalizeWavChannelFormat(int32_t format)
{
    switch (format) {
    case TW_INLINE:
        return TW_INLINE;
    case TW_INTERLEAVED:
    default:
        return TW_INTERLEAVED;
    }
}

template <typename OUT>
inline void copyFromWavFrames(OUT *output, const float *frames, int samples)
{
    if constexpr (std::is_same<OUT, float>::value) {
        std::copy_n(frames, samples, output);
    } else if constexpr (std::is_same<OUT, q15_t>::value) {
        arm_float_to_q15(frames, output, static_cast<uint32_t>(samples));
    } else if constexpr (std::is_same<OUT, sf32>::value) {
        for (int i = 0; i < samples / 2; ++i) {
            output[i].left = frames[2 * i];
            output[i].right = frames[2 * i + 1];
        }
    } else if constexpr (std::is_same<OUT, sq15>::value) {
        arm_float_to_q15(frames, reinterpret_cast<q15_t *>(output), static_cast<uint32_t>(samples));
    }
}

template <typename IN>
inline void copyToWavFrames(float *frames, const IN *input, int samples)
{
    if constexpr (std::is_same<IN, float>::value) {
        std::copy_n(input, samples, frames);
    } else if constexpr (std::is_same<IN, q15_t>::value) {
        arm_q15_to_float(input, frames, static_cast<uint32_t>(samples));
    } else if constexpr (std::is_same<IN, sf32>::value) {
        for (int i = 0; i < samples / 2; ++i) {
            frames[2 * i] = input[i].left;
            frames[2 * i + 1] = input[i].right;
        }
    } else if constexpr (std::is_same<IN, sq15>::value) {
        arm_q15_to_float(reinterpret_cast<const q15_t *>(input), frames, static_cast<uint32_t>(samples));
    }
}

template <typename T>
inline constexpr bool isSupportedWavType()
{
    return std::is_same<T, float>::value || std::is_same<T, q15_t>::value ||
           std::is_same<T, sf32>::value || std::is_same<T, sq15>::value;
}

} // namespace detail

template <typename OUT, int outputSamples>
class WavSource final : public arm_cmsis_stream::GenericSource<OUT, outputSamples> {
    static_assert(detail::isSupportedWavType<OUT>(), "WavSource supports float, q15_t, sf32, and sq15 samples");

  public:
    template <typename Params>
    WavSource(arm_cmsis_stream::FIFOBase<OUT> &dst, const Params &params)
        : arm_cmsis_stream::GenericSource<OUT, outputSamples>(dst),
          numChannels_(params.num_channels)
    {
        const char *path = params.path != nullptr ? params.path : "";
        opened_ = tinywav_open_read(&wav_, path, detail::normalizeWavChannelFormat(params.channel_format)) == 0;
    }

    ~WavSource()
    {
        if (opened_ && tinywav_isOpen(&wav_)) {
            tinywav_close_read(&wav_);
        }
    }

    int run() final
    {
        if (!opened_ || !tinywav_isOpen(&wav_) || wav_.numChannels <= 0 ||
            wav_.numChannels != numChannels_ || numChannels_ > 2) {
            return CG_BUFFER_ERROR;
        }

        OUT *output = this->getWriteBuffer();
        const int requestedFrames = outputSamples;
        const int remainingFrames = wav_.numFramesInHeader - static_cast<int>(wav_.totalFramesReadWritten);

        if (remainingFrames <= 0) {
            return CG_STOP_SCHEDULER;
        }

        const int framesToRead = std::min(requestedFrames, remainingFrames);
        const int samplesToRead = framesToRead * numChannels_;
        if (static_cast<int>(frameBuffer_.size()) < samplesToRead) {
            frameBuffer_.fill(0.0f);
        }

        const int framesRead = tinywav_read_f(&wav_, frameBuffer_.data(), framesToRead);
        if (framesRead < 0) {
            return CG_BUFFER_UNDERFLOW;
        }
        if (framesRead == 0) {
            return CG_STOP_SCHEDULER;
        }
        if (framesRead != framesToRead) {
            return CG_BUFFER_UNDERFLOW;
        }

        const int samplesRead = framesRead * wav_.numChannels;
        detail::copyFromWavFrames(output, frameBuffer_.data(), samplesRead);
        std::fill(output + framesRead, output + outputSamples, OUT{});

        return CG_SUCCESS;
    }

  private:
    TinyWav wav_{};
    int32_t numChannels_;
    std::array<float, outputSamples * 2> frameBuffer_{};
    bool opened_ = false;
};

} // namespace cmsis_stream_nodes

using cmsis_stream_nodes::WavSource;
using cmsis_stream_nodes::WavSourceParams;
