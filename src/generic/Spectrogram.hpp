#pragma once


#include "cg_enums.h"
#include "EventQueue.hpp"
#include "StreamNode.hpp"
#include "GenericNodes.hpp"
#include "arm_math_types.h"
#include "dsp/basic_math_functions.h"
#include "dsp/complex_math_functions.h"
#include <cstring>

using namespace arm_cmsis_stream;

template <typename IN, int inputSize>
class Spectrogram;

template <int inputSamples>
class Spectrogram<cf32, inputSamples>
    : public GenericSink<cf32, inputSamples>
{
  public:
    Spectrogram(FIFOBase<cf32> &src,EventQueue *queue)
        : GenericSink<cf32, inputSamples>(src),ev0(queue)
    {
        mag = new float32_t[inputSamples >> 1];
    };

    ~Spectrogram()
    {
        delete[] mag;
    }


    int run() final
    {
        const int magSamples = inputSamples >> 1;
        cf32 *in = this->getReadBuffer();

        //arm_scale_f32((float32_t*)in, 4.0f, (float32_t*)in, inputSamples);

        // We keep half of the complex FFT spectrum
        arm_cmplx_mag_f32((float32_t *)in, mag, magSamples);


        float di = 1.0f * CONFIG_NB_BINS / ((float)magSamples);
        // float scale = 1.0f * FFT_SIZE / 2 / NB_BIN;
        float k = 0;
        memset(bins, 0, sizeof(bins));

        for (int i = 0; i < magSamples; i++)
        {
            if (k < CONFIG_NB_BINS)
                bins[(int)k] += mag[i];
            k += di;
        }

        for (int i = 0; i < CONFIG_NB_BINS; i++)
        {
            //   bins[i] *= scale;
            if (bins[i] > 1.0f)
                bins[i] = 1.0f;
            if (bins[i] < 0.0f)
                bins[i] = 0.0f;
        }

        

        UniquePtr<float> tensorData(CONFIG_NB_BINS);
        memcpy(tensorData.get(), bins, sizeof(bins));

        // Spectrogram frames have lower priority than video frames and may be delayed
        // by video frame processing.
        // To avoid an overflow of the event queue, spectrogram events are set to
        // live for 40 ms only.
        // So old spectrogram events are discarded by the event queue and not sent to the display
        // node.
        TensorPtr<float> t = TensorPtr<float>::create_with(CONFIG_NB_BINS,std::move(tensorData));
        
        bool status = ev0.sendAsyncWithTTL(kNormalPriority, kValue, 40, std::move(t)); // Send the event to the subscribed nodes

        if (!status)
        {
            CMSISSTREAM_LOG_ERR("Failed to send spectrogram event\n");
        }
        



        return (CG_SUCCESS);
    };

    void subscribe(int outputPort, StreamNode &dst, int dstPort)
    {
        ev0.subscribe(dst, dstPort);
    }

  protected:
    float32_t *mag;
    float32_t bins[CONFIG_NB_BINS];
    EventOutput ev0;
};