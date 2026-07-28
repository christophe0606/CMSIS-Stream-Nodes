#include "hardware_params.h"
#include "app_params.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/kernel.h>

#if defined(CONFIG_I2S)

#define MICROPHONE_DEVICE DT_ALIAS(i2s_mic)
#define MICROPHONE_SAMPLE_SIZE sizeof(int16_t)
#define MICROPHONE_WORD_SIZE 16U
#define MICROPHONE_CHANNELS 2U
#define MICROPHONE_SAMPLES                                                                    \
    (MIC_BLOCK_SIZE / (MIC_CHANNELS * MICROPHONE_SAMPLE_SIZE))
#define MICROPHONE_BUFFER_COUNT 2U
#define MICROPHONE_BUFFER_SIZE \
    (MICROPHONE_CHANNELS * MICROPHONE_SAMPLES * MICROPHONE_SAMPLE_SIZE)

BUILD_ASSERT((MIC_BLOCK_SIZE % (MIC_CHANNELS * MICROPHONE_SAMPLE_SIZE)) == 0,
             "MIC_BLOCK_SIZE must contain a whole number of audio frames");
BUILD_ASSERT((MICROPHONE_BUFFER_SIZE % 4) == 0,
             "The I2S buffer size must be a multiple of 4");

K_MEM_SLAB_DEFINE_STATIC(microphone_mem_slab,
                         MICROPHONE_BUFFER_SIZE,
                         MICROPHONE_BUFFER_COUNT,
                         4);

static const struct device *const microphone_device = DEVICE_DT_GET(MICROPHONE_DEVICE);

#endif

int hardware_params_init(HardwareParams *params)
{
    if (params == NULL) {
        return -1;
    }

    *params = (HardwareParams){0};

#if defined(CONFIG_I2S)
    if (!device_is_ready(microphone_device)) {
        return -2;
    }

    const struct i2s_config config = {
        .word_size = MICROPHONE_WORD_SIZE,
        .channels = MICROPHONE_CHANNELS,
        .format = I2S_FMT_DATA_FORMAT_I2S,
        .options = I2S_OPT_FRAME_CLK_MASTER | I2S_OPT_BIT_CLK_MASTER,
        .frame_clk_freq = MIC_SAMPLE_RATE,
        .mem_slab = &microphone_mem_slab,
        .block_size = MICROPHONE_BUFFER_SIZE,
        .timeout = SYS_FOREVER_MS,
    };

    const int result = i2s_configure(microphone_device, I2S_DIR_RX, &config);
    if (result < 0) {
        return result;
    }

    params->microphone_device = microphone_device;
    params->microphone_mem_slab = &microphone_mem_slab;
    params->microphone_sample_rate = MIC_SAMPLE_RATE;
    params->microphone_num_channels = MICROPHONE_CHANNELS;
#endif

    return 0;
}

void hardware_params_uninit(HardwareParams *params)
{
#if defined(CONFIG_I2S)
    if ((params != NULL) && (params->microphone_device != NULL)) {
        (void)i2s_trigger(params->microphone_device, I2S_DIR_RX, I2S_TRIGGER_DROP);
        params->microphone_device = NULL;
        params->microphone_mem_slab = NULL;
    }
#else
    (void)params;
#endif
}
