#pragma once


using namespace arm_cmsis_stream;

class SpectrogramTextDisplay : public StreamNode
{
    static constexpr uint16_t refresh = 40; // ms


      public:
	SpectrogramTextDisplay() : StreamNode()
	{
	}

	virtual ~SpectrogramTextDisplay() {};
	

    
    void drawSpectrogram(TensorPtr<float> &&s)
    {
        bool lockError;
        s.lock_shared(lockError, [this](const Tensor<float> &tensor)
        {
           
                if (tensor.dims[0] == CONFIG_NB_BINS)
                {
                    const float *buf = tensor.buffer();
                    
                    for (int i = 0; i < CONFIG_NB_BINS; i++)
                    {
                       
                    }   
                } 
        });
    }



    cg_status processEvent(int dstPort, Event &&evt) final override
    {
        if (evt.event_id == kValue)
        {
            if (dstPort == 0)
            {
                if (evt.wellFormed<TensorPtr<float>>())
                {
                    evt.apply<TensorPtr<float>>(&SpectrogramTextDisplay::drawSpectrogram, *this);
                }
            }

           
        }
        return CG_SUCCESS;
        
    }
    

};
