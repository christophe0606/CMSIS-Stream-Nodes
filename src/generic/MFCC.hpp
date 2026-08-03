/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        MFCC.h
 * Description:  Node for CMSIS-DSP MFCC
 *
 *
 * Target Processor: Cortex-M and Cortex-A cores
 * --------------------------------------------------------------------
 *
 * Copyright (C) 2021-2023 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "GenericNodes.hpp"
#include "dsp/transform_functions.h"
#include <vector>

extern "C"
{
#include "kws_mfcc.h"
}

using namespace arm_cmsis_stream;

template <typename IN, int inputSize, typename OUT, int outputSize>
class MFCC;

/*

The MFCC configuration data has to be generated with the CMSIS-DSP Scripts/GenMFCCDataForCPP.py.
It is using a yaml file to describe the configuration

*/

/*

The CMSIS-DSP MFCC F32

*/
template <>
class MFCC<float32_t, 640, float32_t, 10> : public GenericNode<float32_t, 640, float32_t, 10>
{
  public:
    MFCC(FIFOBase<float32_t> &src, FIFOBase<float32_t> &dst)
        : GenericNode<float32_t, 640, float32_t, 10>(src, dst)
    {
        arm_status status = arm_mfcc_init_1024_f32(&mfccConfig, 40, 10,
                                                   mfcc_dct_coefs_kws_f32,
                                                   mfcc_filter_pos_kws_f32,
                                                   mfcc_filter_len_kws_f32,
                                                   mfcc_filter_coefs_kws_f32,
                                                   mfcc_window_coefs_kws_f32);

        if (status != ARM_MATH_SUCCESS)
        {
            CMSISSTREAM_LOG_ERR("MFCC init error\n");
        }
#if defined(ARM_MFCC_CFFT_BASED)
        memory.resize(2 * 1024);
        paddedInput.resize(2 * 1024);
#else
        memory.resize(1024 + 2);
        paddedInput.resize(1024 + 2);
#endif
    };


    int run() final
    {
        float32_t *a = this->getReadBuffer();
        float32_t *b = this->getWriteBuffer();
        
        memcpy(paddedInput.data(), a, 640 * sizeof(float32_t));
        memset(paddedInput.data() + 640, 0, (1024 - 640) * sizeof(float32_t));
        arm_mfcc_f32(&mfccConfig, paddedInput.data(), b, memory.data());

        return (CG_SUCCESS);
    };

    arm_mfcc_instance_f32 mfccConfig;
    std::vector<float32_t> memory;
    std::vector<float32_t> paddedInput;
};
