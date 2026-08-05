#pragma once

#include "cg_enums.h"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"

#include "arm_math_types.h"

#include "dsp/transform_functions.h"
#include <cstring>

using namespace arm_cmsis_stream;

template <typename IN, int inputSize,
          typename OUT, int outputSize>
class CFFT;

template <int inputSamples>
class CFFT<cf32, inputSamples, cf32, inputSamples> : public GenericNode<cf32, inputSamples, cf32, inputSamples>
{
  public:
    CFFT(FIFOBase<cf32> &src, FIFOBase<cf32> &dst)
        : GenericNode<cf32, inputSamples, cf32, inputSamples>(src, dst)
    {
        if constexpr (inputSamples == 128)
            arm_cfft_init_128_f32(&varInstCfftF32);
        else if constexpr (inputSamples == 256)
            arm_cfft_init_256_f32(&varInstCfftF32);
        else if constexpr (inputSamples == 512)
            arm_cfft_init_512_f32(&varInstCfftF32);
        else if constexpr (inputSamples == 1024)
            arm_cfft_init_1024_f32(&varInstCfftF32);
        else if constexpr (inputSamples == 2048)
            arm_cfft_init_2048_f32(&varInstCfftF32);
        else
            static_assert("Unsupported FFT size");
    };

    int run() final
    {
        cf32 *o = this->getWriteBuffer();
        cf32 *in = this->getReadBuffer();

        memcpy(o, in, sizeof(cf32) * inputSamples);
        arm_cfft_f32(&varInstCfftF32, (float32_t *)o, 0, 1);

        return (CG_SUCCESS);
    };

  protected:
    arm_cfft_instance_f32 varInstCfftF32;
};