#pragma once

#include "GenericNodes.hpp"
#include "app_params.h"
#include "cg_enums.h"
#include "datatypes.h"
#include "dsp/support_functions.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <type_traits>

namespace cmsis_stream_nodes {

namespace detail {

template <typename OUT>
inline constexpr bool isSupportedMicrophoneType()
{
    return std::is_same<OUT, float>::value || std::is_same<OUT, q15_t>::value ||
           std::is_same<OUT, sf32>::value || std::is_same<OUT, sq15>::value;
}

template <typename OUT>
inline constexpr int microphoneTypeChannels()
{
    if constexpr (std::is_same<OUT, sf32>::value || std::is_same<OUT, sq15>::value) {
        return 2;
    }
    return 1;
}

template <typename OUT>
inline void copyFromMicrophoneFrames(OUT *output, const float *frames, int frameCount)
{
    if constexpr (std::is_same<OUT, float>::value) {
        std::copy_n(frames, frameCount, output);
    } else if constexpr (std::is_same<OUT, q15_t>::value) {
        arm_float_to_q15(frames, output, static_cast<uint32_t>(frameCount));
    } else if constexpr (std::is_same<OUT, sf32>::value) {
        for (int i = 0; i < frameCount; ++i) {
            output[i].left = frames[2 * i];
            output[i].right = frames[2 * i + 1];
        }
    } else if constexpr (std::is_same<OUT, sq15>::value) {
        arm_float_to_q15(reinterpret_cast<const float *>(frames),
                         reinterpret_cast<q15_t *>(output),
                         static_cast<uint32_t>(frameCount * 2));
    }
}

} // namespace detail

template <typename OUT, int outputSamples>
class MicrophoneSource final : public arm_cmsis_stream::GenericSource<OUT, outputSamples>,
                               public ContextSwitch {
    static_assert(detail::isSupportedMicrophoneType<OUT>(),
                  "MicrophoneSource supports float, q15_t, sf32, and sq15 samples");

  public:
    MicrophoneSource(arm_cmsis_stream::FIFOBase<OUT> &dst, const MicrophoneSourceParams &params)
        : arm_cmsis_stream::GenericSource<OUT, outputSamples>(dst),
          hw_(params.hw_),
          numChannels_(params.num_channels)
    {
    }

    int run() final
    {
        constexpr int expectedChannels = detail::microphoneTypeChannels<OUT>();
        if (numChannels_ != expectedChannels || hw_.microphone_num_channels != expectedChannels) {
            return CG_BUFFER_ERROR;
        }

        OUT *output = this->getWriteBuffer();
        if (!started_) {
            if (hardware_microphone_start(&hw_) != 0) {
                std::fill(output, output + outputSamples, OUT{});
                return CG_BUFFER_UNDERFLOW;
            }
            started_ = true;
        }

        const int framesRead = hardware_microphone_read(&hw_, frameBuffer_.data(), outputSamples);
        if (framesRead < 0) {
            std::fill(output, output + outputSamples, OUT{});
            return CG_BUFFER_UNDERFLOW;
        }

        detail::copyFromMicrophoneFrames(output, frameBuffer_.data(), framesRead);
        std::fill(output + framesRead, output + outputSamples, OUT{});

        return CG_SUCCESS;
    }

    int pause() final
    {
        started_ = false;
        return hardware_microphone_pause(&hw_);
    }

    int resume() final
    {
        const int result = hardware_microphone_resume(&hw_);
        started_ = result == 0;
        return result;
    }

  private:
    HardwareParams hw_{};
    int32_t numChannels_;
    std::array<float, outputSamples * detail::microphoneTypeChannels<OUT>()> frameBuffer_{};
    bool started_ = false;
};

} // namespace cmsis_stream_nodes

using cmsis_stream_nodes::MicrophoneSource;
