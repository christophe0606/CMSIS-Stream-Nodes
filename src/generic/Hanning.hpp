#pragma once

#include "cg_enums.h"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
#include "dsp/basic_math_functions.h"
#include "dsp/window_functions.h"
#include <cstring>


using namespace arm_cmsis_stream;

template <typename IN, int inputSize, typename OUT, int outputSize>
class Hanning;

template <int inputSamples,int outputSamples>
class Hanning<float32_t, inputSamples, float32_t, outputSamples> : public GenericNode<float32_t, inputSamples, float32_t, outputSamples>
{
  public:
    Hanning(FIFOBase<float32_t> &src, FIFOBase<float32_t> &dst)
        : GenericNode<float32_t, inputSamples, float32_t, outputSamples>(src, dst)
    {
        window = new float32_t[inputSamples];
        arm_hanning_f32(window, inputSamples);
    };

    ~Hanning()
    {
        delete[] window;
    }

    
    int run() final
    {
        float32_t *in = this->getReadBuffer();
        float32_t *out = this->getWriteBuffer();

        memset(out, 0, sizeof(float32_t) * outputSamples);
        arm_mult_f32(in, window, out + offset, inputSamples);

        return (CG_SUCCESS);
    };

  protected:
    float32_t *window;
    static constexpr int offset = (outputSamples - inputSamples) >> 1;
};