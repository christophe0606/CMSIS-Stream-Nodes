#pragma once

#include "cg_enums.h"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
#include "dsp/basic_math_functions.h"
#include <cstring>

using namespace arm_cmsis_stream;

template <typename IN, int inputSize, typename OUT, int outputSize> class Gain;

template <int inputSamples>
class Gain<float32_t, inputSamples, float32_t, inputSamples>
	: public GenericNode<float32_t, inputSamples, float32_t, inputSamples>
{
      public:
	Gain(FIFOBase<float32_t> &src, FIFOBase<float32_t> &dst,  const GainParams &params)
		: GenericNode<float32_t, inputSamples, float32_t, inputSamples>(src, dst),
		  gain_(params.gain) {

		  };

	
	int run() final
	{
		float32_t *in = this->getReadBuffer();
		float32_t *out = this->getWriteBuffer();

		arm_scale_f32(in, gain_, out, inputSamples);

		return (CG_SUCCESS);
	};

      protected:
	float32_t gain_;
};

template <int inputSamples>
class Gain<sf32, inputSamples, sf32, inputSamples>
	: public GenericNode<sf32, inputSamples, sf32, inputSamples>
{
      public:
	Gain(FIFOBase<sf32> &src, FIFOBase<sf32> &dst,  const GainParams &params)
		: GenericNode<sf32, inputSamples, sf32, inputSamples>(src, dst), gain_(params.gain) {

		  };



	int run() final
	{
		float32_t *in = (float32_t *)this->getReadBuffer();
		float32_t *out = (float32_t *)this->getWriteBuffer();

		arm_scale_f32(in, gain_, out, 2 * inputSamples);

		return (CG_SUCCESS);
	};

      protected:
	float32_t gain_;
};

template <int inputSamples>
class Gain<q15_t, inputSamples, q15_t, inputSamples>
	: public GenericNode<q15_t, inputSamples, q15_t, inputSamples>
{
      public:
	Gain(FIFOBase<q15_t> &src, FIFOBase<q15_t> &dst, float32_t gain = 1.0f)
		: GenericNode<q15_t, inputSamples, q15_t, inputSamples>(src, dst)
	{
		while (fabsf(gain) >= 1.0f) {
			gain /= 2.0f;
			shift_++;
		}
		gain_ = (q15_t)(gain * 32768.0f);
	};


	int run() final
	{
		q15_t *in = this->getReadBuffer();
		q15_t *out = this->getWriteBuffer();

		arm_scale_q15(in, gain_, shift_, out, inputSamples);

		return (CG_SUCCESS);
	};

      protected:
	q15_t gain_;
	int shift_ = 0;
};

template <int inputSamples>
class Gain<sq15, inputSamples, sq15, inputSamples>
	: public GenericNode<sq15, inputSamples, sq15, inputSamples>
{
      public:
	Gain(FIFOBase<sq15> &src, FIFOBase<sq15> &dst,  const GainParams &params)
		: GenericNode<sq15, inputSamples, sq15, inputSamples>(src, dst)
	{
		float32_t gain = params.gain;
		while (fabsf(gain) >= 1.0f) {
			gain /= 2.0f;
			shift_++;
		}
		gain_ = (q15_t)(gain * 32768.0f);

	};


	int run() final
	{
		q15_t *in = (q15_t *)this->getReadBuffer();
		q15_t *out = (q15_t *)this->getWriteBuffer();

		arm_scale_q15(in, gain_, shift_, out, 2 * inputSamples);

		return (CG_SUCCESS);
	};

      protected:
	q15_t gain_;
	int shift_ = 0;
};