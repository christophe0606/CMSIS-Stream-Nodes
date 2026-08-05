#pragma once

#include "GenericNodes.hpp"
#include "StreamNode.hpp"
#include "cg_enums.h"
#include <type_traits>

#include "dsp/basic_math_functions.h"

using namespace arm_cmsis_stream;

template <typename IN, int inputSize,
          typename OUT, int outputSize>
class InterleavedStereoToMono;

template <typename IN, typename OUT, int inputSamples>
class InterleavedStereoToMono<IN, inputSamples, OUT, inputSamples> : public GenericNode<IN, inputSamples, OUT, inputSamples>
{

  public:
    InterleavedStereoToMono(FIFOBase<IN> &src, FIFOBase<OUT> &dst)
        : GenericNode<IN, inputSamples, OUT, inputSamples>(src, dst) {};


    int run() final
    {
        OUT *dst = this->getWriteBuffer();
        IN *src = this->getReadBuffer();

        if constexpr ((std::is_same_v<IN, sf32>) && (std::is_same_v<OUT, float>))
        {
           for (int i = 0; i < inputSamples; i++)
           {
                dst[i] = src[i].left * 0.5f + src[i].right * 0.5f;
           }
        }
        else if constexpr ((std::is_same_v<IN, sq15>) && (std::is_same_v<OUT, q15_t>))
        {
             for (int i = 0; i < inputSamples; i++)
             {
                  dst[i] = (q15_t)((((int32_t)src[i].left + (int32_t)src[i].right) >> 1));
             }
        }
        else 
        {
           static_assert(false, "Unsupported type");
        }

        return (CG_SUCCESS);
    };
};