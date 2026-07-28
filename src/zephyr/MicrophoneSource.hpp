#pragma once

#include "GenericNodes.hpp"
#include "app_params.h"
#include "cg_enums.h"
#include "common/MicrophoneSource.hpp"

#include <zephyr/drivers/i2s.h>
#include <zephyr/kernel.h>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace cmsis_stream_nodes {

template <typename OUT, int outputSamples>
class MicrophoneSource final : public arm_cmsis_stream::GenericSource<OUT, outputSamples>,
                               public ContextSwitch {
    static_assert(detail::isSupportedMicrophoneType<OUT>(),
                  "MicrophoneSource supports float, q15_t, sf32, and sq15 samples");
#if defined(CONFIG_I2S)
    static_assert(CONFIG_I2S_SAMPLES == outputSamples,
                  "MicrophoneSource output size must match CONFIG_I2S_SAMPLES");
#endif

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

        if (!started_.load()) {
            const int result =
                i2s_trigger(hw_.microphone_device, I2S_DIR_RX, I2S_TRIGGER_START);
            if (result < 0) {
                fillOutputWithSilence();
                return CG_INIT_FAILURE;
            }
            started_.store(true);
        }

        void *buffer = nullptr;
        size_t size = 0;
        const int result = i2s_read(hw_.microphone_device, &buffer, &size);
        if ((result != 0) || (buffer == nullptr)) {
            if (buffer != nullptr) {
                k_mem_slab_free(hw_.microphone_mem_slab, buffer);
            }
            fillOutputWithSilence();
            stopAudio();
            return CG_BUFFER_UNDERFLOW;
        }

        const size_t bytesPerFrame =
            static_cast<size_t>(hw_.microphone_num_channels) * sizeof(q15_t);
        const int framesRead = static_cast<int>(
            std::min<size_t>(size / bytesPerFrame, static_cast<size_t>(outputSamples)));
        OUT *output = this->getWriteBuffer();
        detail::copyFromInterleavedPcm16(output,
                                         static_cast<const q15_t *>(buffer),
                                         framesRead,
                                         hw_.microphone_num_channels);
        std::fill(output + framesRead, output + outputSamples, OUT{});
        k_mem_slab_free(hw_.microphone_mem_slab, buffer);
        return CG_SUCCESS;
    }

    int pause() final
    {
        if (!started_.load()) {
            return 0;
        }
        const int result =
            i2s_trigger(hw_.microphone_device, I2S_DIR_RX, I2S_TRIGGER_STOP);
        started_.store(false);
        return result;
    }

    int resume() final
    {
        started_.store(false);
        return 0;
    }

  private:
    bool hardwareReady() const
    {
        return hw_.microphone_device != nullptr && hw_.microphone_mem_slab != nullptr &&
               hw_.microphone_num_channels >= detail::microphoneTypeChannels<OUT>() &&
               numChannels_ == detail::microphoneTypeChannels<OUT>();
    }

    void fillOutputWithSilence()
    {
        OUT *output = this->getWriteBuffer();
        std::fill(output, output + outputSamples, OUT{});
    }

    void stopAudio()
    {
        if (hw_.microphone_device != nullptr) {
            (void)i2s_trigger(hw_.microphone_device, I2S_DIR_RX, I2S_TRIGGER_DROP);
        }
        started_.store(false);
    }

    HardwareParams hw_{};
    int32_t numChannels_;
    std::atomic<bool> started_{false};
};

} // namespace cmsis_stream_nodes

using cmsis_stream_nodes::MicrophoneSource;
