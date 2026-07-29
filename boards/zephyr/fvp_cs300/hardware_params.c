#include "hardware_params.h"
#include "app_params.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/kernel.h>

#ifndef MIC_SAMPLE_RATE
#error "MIC_SAMPLE_RATE must be defined by the application"
#endif

#ifndef MIC_CHANNELS
#error "MIC_CHANNELS must be defined by the application"
#endif

#ifndef MIC_BLOCK_SIZE
#error "MIC_BLOCK_SIZE must be defined by the application"
#endif

#ifndef MIC_SAMPLE_SIZE
#error "MIC_SAMPLE_SIZE must be defined by the application"
#endif

#if defined(CONFIG_I2S)

#define MICROPHONE_DEVICE DT_ALIAS(i2s_mic)
#define MICROPHONE_BUFFER_COUNT 2

BUILD_ASSERT(MIC_SAMPLE_RATE > 0, "MIC_SAMPLE_RATE must be greater than zero");
BUILD_ASSERT(MIC_SAMPLE_SIZE == 16, "The VSI microphone path supports 16-bit PCM only");
BUILD_ASSERT((MIC_CHANNELS == 1) || (MIC_CHANNELS == 2),
             "The microphone source supports one or two channels");
BUILD_ASSERT((MIC_BLOCK_SIZE % (MIC_CHANNELS * (MIC_SAMPLE_SIZE / 8))) == 0,
             "MIC_BLOCK_SIZE must contain a whole number of audio frames");
BUILD_ASSERT((MIC_BLOCK_SIZE % 4) == 0, "MIC_BLOCK_SIZE must be a multiple of 4");

K_MEM_SLAB_DEFINE_STATIC(microphone_mem_slab,
                         MIC_BLOCK_SIZE,
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
        .word_size = MIC_SAMPLE_SIZE,
        .channels = MIC_CHANNELS,
        .format = I2S_FMT_DATA_FORMAT_I2S,
        .options = I2S_OPT_FRAME_CLK_MASTER | I2S_OPT_BIT_CLK_MASTER,
        .frame_clk_freq = MIC_SAMPLE_RATE,
        .mem_slab = &microphone_mem_slab,
        .block_size = MIC_BLOCK_SIZE,
        .timeout = SYS_FOREVER_MS,
    };

    const int result = i2s_configure(microphone_device, I2S_DIR_RX, &config);
    if (result < 0) {
        return result;
    }

    params->microphone_device = microphone_device;
    params->microphone_mem_slab = &microphone_mem_slab;
    params->microphone_sample_rate = MIC_SAMPLE_RATE;
    params->microphone_num_channels = MIC_CHANNELS;
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
