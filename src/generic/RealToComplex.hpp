#pragma once

#include "cg_enums.h"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"

using namespace arm_cmsis_stream;

template <typename IN, int inputSize,
          typename OUT, int outputSize>
class RealToComplex;

template <int inputSamples>
class RealToComplex<float, inputSamples, cf32, inputSamples> : public GenericNode<float, inputSamples, cf32, inputSamples>
{
  public:
    RealToComplex(FIFOBase<float> &src, FIFOBase<cf32> &dst)
        : GenericNode<float, inputSamples, cf32, inputSamples>(src, dst) {};

    
    int run() final
    {
        cf32 *o = this->getWriteBuffer();
        float *in = this->getReadBuffer();
        for (int i = 0; i < inputSamples; i++)
        {
            o[i].real = in[i];
            o[i].imag = 0;
        }

        return (CG_SUCCESS);
    };
};