#pragma once

#include "cg_enums.h"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"


#include "dsp/support_functions.h"

extern "C"
{
    #include "datatypes.h"
}

using namespace arm_cmsis_stream;

template <typename IN, int inputSize,
          typename OUT, int outputSize>
class Convert;

template <int inputSamples>
class Convert<sq15, inputSamples, sf32, inputSamples> : public GenericNode<sq15, inputSamples, sf32, inputSamples>
{
  public:
    Convert(FIFOBase<sq15> &src, FIFOBase<sf32> &dst)
        : GenericNode<sq15, inputSamples, sf32, inputSamples>(src, dst) {};

    
    int run() final
    {
        sf32 *o = this->getWriteBuffer();
        sq15 *in = this->getReadBuffer();
        arm_q15_to_float((q15_t *)in, (float32_t *)o, 2 * inputSamples);

        return (CG_SUCCESS);
    };
};

template <int inputSamples>
class Convert<sf32, inputSamples, sq15, inputSamples> : public GenericNode<sf32, inputSamples, sq15, inputSamples>
{
  public:
    Convert(FIFOBase<sf32> &src, FIFOBase<sq15> &dst)
        : GenericNode<sf32, inputSamples, sq15, inputSamples>(src, dst) {};


    int run() final
    {
        sq15 *o = this->getWriteBuffer();
        sf32 *in = this->getReadBuffer();
        arm_float_to_q15((float32_t *)in, (q15_t *)o, 2 * inputSamples);

        return (CG_SUCCESS);
    };
};

template <int inputSamples>
class Convert<q15_t, inputSamples, float, inputSamples> : public GenericNode<q15_t, inputSamples, float, inputSamples>
{
  public:
    Convert(FIFOBase<q15_t> &src, FIFOBase<float> &dst)
        : GenericNode<q15_t, inputSamples, float, inputSamples>(src, dst) {};


    int run() final
    {
        float *o = this->getWriteBuffer();
        q15_t *in = this->getReadBuffer();
        arm_q15_to_float((q15_t *)in, (float32_t *)o, inputSamples);

        return (CG_SUCCESS);
    };
};