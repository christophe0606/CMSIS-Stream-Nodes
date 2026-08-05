#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>


using namespace arm_cmsis_stream;

class SpectrogramTextDisplay : public StreamNode
{
    static uint32_t crc32(const uint8_t *data, size_t length)
    {
        uint32_t crc = 0xFFFFFFFFu;
        for (size_t i = 0; i < length; ++i)
        {
            crc ^= data[i];
            for (int bit = 0; bit < 8; ++bit)
            {
                const uint32_t mask = 0u - (crc & 1u);
                crc = (crc >> 1) ^ (0xEDB88320u & mask);
            }
        }
        return ~crc;
    }

    static void putU16(uint8_t *dst, uint16_t value)
    {
        dst[0] = static_cast<uint8_t>(value);
        dst[1] = static_cast<uint8_t>(value >> 8);
    }

    static void putU32(uint8_t *dst, uint32_t value)
    {
        dst[0] = static_cast<uint8_t>(value);
        dst[1] = static_cast<uint8_t>(value >> 8);
        dst[2] = static_cast<uint8_t>(value >> 16);
        dst[3] = static_cast<uint8_t>(value >> 24);
    }

    uint32_t sequence = 0;

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
                    static_assert(sizeof(float) == 4, "Protocol requires 32-bit floats");
                    static const uint8_t magic[8] =
                        {0xA5, 0x5A, 0xC3, 0x3C, 'S', 'P', 'G', '1'};

                    constexpr size_t payloadSize = CONFIG_NB_BINS * sizeof(float);
                    constexpr size_t frameSize = 8 + 2 + 4 + payloadSize + 4;
                    std::array<uint8_t, frameSize> frame;

                    std::memcpy(frame.data(), magic, sizeof(magic));
                    putU16(frame.data() + 8, CONFIG_NB_BINS);
                    putU32(frame.data() + 10, sequence++);
                    std::memcpy(frame.data() + 14, buf, payloadSize);

                    const uint32_t checksum =
                        crc32(frame.data() + 8, 2 + 4 + payloadSize);
                    putU32(frame.data() + 14 + payloadSize, checksum);

                    std::fwrite(frame.data(), 1, frame.size(), stdout);
                    std::fflush(stdout);
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
