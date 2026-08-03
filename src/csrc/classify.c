
#include "arm_vec_math.h"
#include "dsp/fast_math_functions.h"
#include "dsp/statistics_functions.h"


void node_softmax(float *in, size_t blockSize)
	{
		float32_t maxVal;
		const float32_t *pIn;
		int32_t blkCnt;
		float32_t accum = 0.0f;
		float32_t tmp;

		arm_max_no_idx_f32((float32_t *)in, blockSize, &maxVal);

		blkCnt = blockSize;
		pIn = in;

		f32x4_t vSum = vdupq_n_f32(0.0f);
		blkCnt = blockSize >> 2;
		while (blkCnt > 0) {
			f32x4_t vecIn = vld1q(pIn);
			f32x4_t vecExp;

			vecExp = vexpq_f32(vsubq_n_f32(vecIn, maxVal));

			vSum = vaddq_f32(vSum, vecExp);

			/*
			 * Decrement the blockSize loop counter
			 * Advance vector source and destination pointers
			 */
			pIn += 4;
			blkCnt--;
		}

		/* sum + log */
		accum = vecAddAcrossF32Mve(vSum);

		blkCnt = blockSize & 0x3;
		while (blkCnt > 0) {
			tmp = *pIn++;
			accum += expf(tmp - maxVal);
			blkCnt--;
		}

		tmp = 1.0f / accum;
		arm_scale_f32((const float32_t *)in, tmp, in, blockSize);
	}