#pragma once

#include "GenericNodes.hpp"
#include "app_params.h"
#include "cg_enums.h"
#include "cmsis_os2.h"
#include "vstream_audio_in.h"
#include "datatypes.h"
#include "dsp/support_functions.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
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

template <typename OUT, int outputSamples>
inline void copyFromPcm16(OUT *output, const q15_t *input, int frameCount)
{
    if constexpr (std::is_same<OUT, float>::value) {
        arm_q15_to_float(input, output, static_cast<uint32_t>(frameCount));
    } else if constexpr (std::is_same<OUT, q15_t>::value) {
        std::copy_n(input, frameCount, output);
    } else if constexpr (std::is_same<OUT, sf32>::value) {
        std::array<float, outputSamples * 2> converted{};
        arm_q15_to_float(input, converted.data(), static_cast<uint32_t>(frameCount * 2));
        for (int i = 0; i < frameCount; ++i) {
            output[i].left = converted[2 * i];
            output[i].right = converted[2 * i + 1];
        }
    } else if constexpr (std::is_same<OUT, sq15>::value) {
        std::memcpy(output, input, static_cast<size_t>(frameCount) * sizeof(sq15));
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
        if (!hardwareReady()) {
            fillOutputWithSilence();
            return CG_INIT_FAILURE;
        }

        if (!started_) {
            const int startResult = hardware_audio_resume(&hw_);
            if (startResult != 0) {
                fillOutputWithSilence();
                return CG_INIT_FAILURE;
            }
            started_ = true;
        }

        const uint32_t flags = osEventFlagsWait(event(),
                                                HARDWARE_MICROPHONE_FRAME_EVENT |
                                                    HARDWARE_MICROPHONE_OVERFLOW_EVENT,
                                                osFlagsWaitAny,
                                                osWaitForever);
        if ((flags & osFlagsError) != 0U) {
            fillOutputWithSilence();
            return CG_BUFFER_UNDERFLOW;
        }
        if ((flags & HARDWARE_MICROPHONE_OVERFLOW_EVENT) != 0U) {
            fillOutputWithSilence();
            return CG_BUFFER_OVERFLOW;
        }

        const q15_t *input = static_cast<const q15_t *>(driver()->GetBlock());
        if (input == nullptr) {
            fillOutputWithSilence();
            return CG_BUFFER_UNDERFLOW;
        }

        detail::copyFromPcm16<OUT, outputSamples>(this->getWriteBuffer(), input, outputSamples);
        (void)driver()->ReleaseBlock();
        return CG_SUCCESS;
    }

    int pause() final
    {
        if (!started_) {
            return 0;
        }
        started_ = false;
        return hardware_audio_pause(&hw_);
    }

    int resume() final
    {
        started_ = false;
        return 0;
    }

  private:
    static constexpr int expectedChannels = detail::microphoneTypeChannels<OUT>();

    vStreamDriver_t *driver() const
    {
        return static_cast<vStreamDriver_t *>(hw_.microphone_stream);
    }

    osEventFlagsId_t event() const
    {
        return static_cast<osEventFlagsId_t>(hw_.microphone_event);
    }

    bool hardwareReady() const
    {
        return driver() != nullptr && event() != nullptr && numChannels_ == expectedChannels &&
               hw_.microphone_num_channels == expectedChannels;
    }

    void fillOutputWithSilence()
    {
        OUT *output = this->getWriteBuffer();
        std::fill(output, output + outputSamples, OUT{});
    }

    HardwareParams hw_{};
    int32_t numChannels_;
    bool started_ = false;
};

} // namespace cmsis_stream_nodes

using cmsis_stream_nodes::MicrophoneSource;
