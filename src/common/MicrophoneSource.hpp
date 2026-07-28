#pragma once

#include "datatypes.h"

#include <type_traits>

namespace cmsis_stream_nodes::detail {

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
inline void copyFromInterleavedPcm16(OUT *output,
                                     const q15_t *input,
                                     int frameCount,
                                     int inputChannels)
{
    for (int i = 0; i < frameCount; ++i) {
        const q15_t left = input[i * inputChannels];
        if constexpr (std::is_same<OUT, float>::value) {
            output[i] = static_cast<float>(left) / 32768.0f;
        } else if constexpr (std::is_same<OUT, q15_t>::value) {
            output[i] = left;
        } else if constexpr (std::is_same<OUT, sf32>::value) {
            output[i].left = static_cast<float>(left) / 32768.0f;
            output[i].right = static_cast<float>(input[i * inputChannels + 1]) / 32768.0f;
        } else if constexpr (std::is_same<OUT, sq15>::value) {
            output[i].left = left;
            output[i].right = input[i * inputChannels + 1];
        }
    }
}

} // namespace cmsis_stream_nodes::detail
