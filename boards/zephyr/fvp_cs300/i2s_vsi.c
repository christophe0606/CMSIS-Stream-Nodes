/*
 * Copyright (c) 2026 Arm Limited or its affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT arm_vsi_i2s

#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/irq.h>
#include <zephyr/kernel.h>

#define VSI_CONTROL_ENABLE BIT(0)
#define VSI_CONTROL_CONTINUOUS BIT(2)

#define VSI_STATUS_ACTIVE BIT(0)
#define VSI_STATUS_DATA BIT(1)
#define VSI_STATUS_EOS BIT(2)
#define VSI_STATUS_FILE_NAME BIT(3)
#define VSI_STATUS_FILE_VALID BIT(4)

#define VSI_IRQ_TIMER BIT(0)

#define VSI_TIMER_RUN BIT(0)
#define VSI_TIMER_PERIODIC BIT(1)
#define VSI_TIMER_TRIGGER_IRQ BIT(2)
#define VSI_TIMER_TRIGGER_DMA BIT(3)

#define VSI_DMA_ENABLE BIT(0)

#define VSI_RX_QUEUE_LENGTH 4

struct vsi_registers {
	struct {
		volatile uint32_t enable;
		volatile uint32_t set;
		volatile uint32_t clear;
		volatile const uint32_t status;
	} irq;
	uint32_t reserved1[60];
	struct {
		volatile uint32_t control;
		volatile uint32_t interval;
		volatile const uint32_t count;
	} timer;
	uint32_t reserved2[61];
	struct {
		volatile uint32_t control;
		volatile uint32_t address;
		volatile uint32_t block_size;
		volatile uint32_t block_count;
		volatile const uint32_t block_index;
	} dma;
	uint32_t reserved3[59];
	volatile uint32_t regs[64];
};

enum vsi_i2s_state {
	VSI_I2S_NOT_READY,
	VSI_I2S_READY,
	VSI_I2S_RUNNING,
	VSI_I2S_STOPPING,
	VSI_I2S_ERROR,
};

struct vsi_rx_block {
	void *memory;
	size_t size;
};

struct vsi_i2s_config {
	struct vsi_registers *registers;
	const char *source_file;
	void (*irq_configure)(const struct device *dev);
};

struct vsi_i2s_data {
	struct i2s_config rx_config;
	enum vsi_i2s_state state;
	void *active_block;
	bool stop_pending;
	struct k_msgq rx_queue;
	char rx_queue_buffer[VSI_RX_QUEUE_LENGTH * sizeof(struct vsi_rx_block)] __aligned(4);
};

static void vsi_i2s_stream_disable(const struct device *dev)
{
	const struct vsi_i2s_config *config = dev->config;

	config->registers->timer.control = 0U;
	config->registers->dma.control = 0U;
	config->registers->regs[0] = 0U;
}

static void vsi_i2s_free_queued_blocks(struct vsi_i2s_data *data)
{
	struct vsi_rx_block block;

	while (k_msgq_get(&data->rx_queue, &block, K_NO_WAIT) == 0) {
		k_mem_slab_free(data->rx_config.mem_slab, block.memory);
	}
}

static void vsi_i2s_drop(const struct device *dev)
{
	struct vsi_i2s_data *data = dev->data;

	data->state = VSI_I2S_READY;
	data->stop_pending = false;
	vsi_i2s_stream_disable(dev);
	vsi_i2s_free_queued_blocks(data);
}

static int vsi_i2s_prepare_dma_block(const struct device *dev)
{
	const struct vsi_i2s_config *config = dev->config;
	struct vsi_i2s_data *data = dev->data;

	if (data->active_block == NULL) {
		if (k_mem_slab_alloc(data->rx_config.mem_slab,
				     &data->active_block,
				     K_NO_WAIT) != 0) {
			return -ENOMEM;
		}
	}

	config->registers->dma.address = (uint32_t)(uintptr_t)data->active_block;
	config->registers->dma.block_size = data->rx_config.block_size;
	config->registers->dma.block_count = 1U;
	return 0;
}

static void vsi_i2s_fail(const struct device *dev)
{
	struct vsi_i2s_data *data = dev->data;

	data->state = VSI_I2S_ERROR;
	vsi_i2s_stream_disable(dev);
}

static void vsi_i2s_isr(const void *arg)
{
	const struct device *dev = arg;
	const struct vsi_i2s_config *config = dev->config;
	struct vsi_i2s_data *data = dev->data;
	const uint32_t status = config->registers->regs[1];

	config->registers->irq.clear = VSI_IRQ_TIMER;

	if ((data->state != VSI_I2S_RUNNING) && (data->state != VSI_I2S_STOPPING)) {
		return;
	}

	if (((status & VSI_STATUS_DATA) != 0U) && (data->active_block != NULL)) {
		struct vsi_rx_block completed = {
			.memory = NULL,
			.size = data->rx_config.block_size,
		};

		if (k_mem_slab_alloc(data->rx_config.mem_slab,
				     &completed.memory,
				     K_NO_WAIT) != 0) {
			vsi_i2s_fail(dev);
			return;
		}
		memcpy(completed.memory, data->active_block, completed.size);
		if (k_msgq_put(&data->rx_queue, &completed, K_NO_WAIT) != 0) {
			k_mem_slab_free(data->rx_config.mem_slab, completed.memory);
			vsi_i2s_fail(dev);
			return;
		}
	}

	if (data->stop_pending || ((status & VSI_STATUS_EOS) != 0U)) {
		vsi_i2s_stream_disable(dev);
		data->stop_pending = false;
		data->state = VSI_I2S_READY;
		return;
	}

}

static int vsi_i2s_configure(const struct device *dev,
			     enum i2s_dir direction,
			     const struct i2s_config *i2s_config)
{
	const struct vsi_i2s_config *config = dev->config;
	struct vsi_i2s_data *data = dev->data;
	uint32_t source_status;

	if ((direction != I2S_DIR_RX) || (i2s_config == NULL)) {
		return -EINVAL;
	}
	if ((data->state != VSI_I2S_NOT_READY) && (data->state != VSI_I2S_READY)) {
		return -EINVAL;
	}
	if (i2s_config->frame_clk_freq == 0U) {
		if (data->state == VSI_I2S_READY) {
			vsi_i2s_drop(dev);
		}
		memset(&data->rx_config, 0, sizeof(data->rx_config));
		data->state = VSI_I2S_NOT_READY;
		return 0;
	}
	if ((i2s_config->mem_slab == NULL) || (i2s_config->block_size == 0U) ||
	    ((i2s_config->block_size & 3U) != 0U) || (i2s_config->word_size != 16U) ||
	    (i2s_config->channels == 0U)) {
		return -EINVAL;
	}

	data->rx_config = *i2s_config;
	config->registers->timer.control = 0U;
	config->registers->dma.control = 0U;
	config->registers->irq.clear = VSI_IRQ_TIMER;
	config->registers->irq.enable = VSI_IRQ_TIMER;
	config->registers->regs[0] = 0U;
	config->registers->regs[4] = i2s_config->channels;
	config->registers->regs[5] = i2s_config->frame_clk_freq;
	config->registers->regs[6] = i2s_config->word_size;

	for (const char *character = config->source_file; *character != '\0'; ++character) {
		config->registers->regs[3] = (uint8_t)*character;
	}
	config->registers->regs[3] = 0U;
	source_status = config->registers->regs[1];
	if ((source_status & (VSI_STATUS_FILE_NAME | VSI_STATUS_FILE_VALID)) !=
	    (VSI_STATUS_FILE_NAME | VSI_STATUS_FILE_VALID)) {
		memset(&data->rx_config, 0, sizeof(data->rx_config));
		return -EIO;
	}

	data->state = VSI_I2S_READY;
	return 0;
}

static const struct i2s_config *vsi_i2s_config_get(const struct device *dev,
						    enum i2s_dir direction)
{
	struct vsi_i2s_data *data = dev->data;

	if ((direction != I2S_DIR_RX) || (data->state == VSI_I2S_NOT_READY)) {
		return NULL;
	}
	return &data->rx_config;
}

static int vsi_i2s_start(const struct device *dev)
{
	const struct vsi_i2s_config *config = dev->config;
	struct vsi_i2s_data *data = dev->data;
	const uint32_t bytes_per_frame =
		data->rx_config.channels * ((data->rx_config.word_size + 7U) / 8U);
	int result;

	if (data->state != VSI_I2S_READY) {
		return -EIO;
	}
	result = vsi_i2s_prepare_dma_block(dev);
	if (result != 0) {
		return result;
	}

	config->registers->regs[0] = VSI_CONTROL_ENABLE | VSI_CONTROL_CONTINUOUS;
	if ((config->registers->regs[1] & VSI_STATUS_ACTIVE) == 0U) {
		return -EIO;
	}

	config->registers->dma.control = VSI_DMA_ENABLE;
	config->registers->timer.interval =
		((uint64_t)1000000U * (data->rx_config.block_size / bytes_per_frame)) /
		data->rx_config.frame_clk_freq;
	data->state = VSI_I2S_RUNNING;
	config->registers->timer.control = VSI_TIMER_RUN | VSI_TIMER_PERIODIC |
					  VSI_TIMER_TRIGGER_IRQ | VSI_TIMER_TRIGGER_DMA;
	return 0;
}

static int vsi_i2s_trigger(const struct device *dev,
			   enum i2s_dir direction,
			   enum i2s_trigger_cmd command)
{
	struct vsi_i2s_data *data = dev->data;

	if (direction != I2S_DIR_RX) {
		return -EINVAL;
	}

	switch (command) {
	case I2S_TRIGGER_START:
		return vsi_i2s_start(dev);
	case I2S_TRIGGER_STOP:
	case I2S_TRIGGER_DRAIN:
		if (data->state != VSI_I2S_RUNNING) {
			return -EIO;
		}
		data->stop_pending = true;
		data->state = VSI_I2S_STOPPING;
		return 0;
	case I2S_TRIGGER_DROP:
		if (data->state == VSI_I2S_NOT_READY) {
			return -EIO;
		}
		vsi_i2s_drop(dev);
		return 0;
	case I2S_TRIGGER_PREPARE:
		if (data->state != VSI_I2S_ERROR) {
			return -EIO;
		}
		vsi_i2s_drop(dev);
		return 0;
	default:
		return -EINVAL;
	}
}

static int vsi_i2s_read(const struct device *dev, void **memory_block, size_t *size)
{
	struct vsi_i2s_data *data = dev->data;
	struct vsi_rx_block block;
	k_timeout_t timeout;
	int result;

	if ((memory_block == NULL) || (size == NULL) || (data->state == VSI_I2S_NOT_READY)) {
		return -EIO;
	}
	if (data->rx_config.timeout == SYS_FOREVER_MS) {
		timeout = K_FOREVER;
	} else if (data->rx_config.timeout == 0) {
		timeout = K_NO_WAIT;
	} else {
		timeout = K_MSEC(data->rx_config.timeout);
	}

	result = k_msgq_get(&data->rx_queue, &block, timeout);
	if (result != 0) {
		if ((data->state == VSI_I2S_ERROR) || (data->state == VSI_I2S_NOT_READY)) {
			return -EIO;
		}
		return data->rx_config.timeout == 0 ? -EBUSY : -EAGAIN;
	}

	*memory_block = block.memory;
	*size = block.size;
	return 0;
}

static int vsi_i2s_write(const struct device *dev, void *memory_block, size_t size)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(memory_block);
	ARG_UNUSED(size);
	return -ENOTSUP;
}

static DEVICE_API(i2s, vsi_i2s_driver_api) = {
	.configure = vsi_i2s_configure,
	.config_get = vsi_i2s_config_get,
	.trigger = vsi_i2s_trigger,
	.read = vsi_i2s_read,
	.write = vsi_i2s_write,
};

static int vsi_i2s_init(const struct device *dev)
{
	const struct vsi_i2s_config *config = dev->config;
	struct vsi_i2s_data *data = dev->data;

	k_msgq_init(&data->rx_queue,
		    data->rx_queue_buffer,
		    sizeof(struct vsi_rx_block),
		    VSI_RX_QUEUE_LENGTH);
	data->state = VSI_I2S_NOT_READY;
	config->registers->timer.control = 0U;
	config->registers->dma.control = 0U;
	config->registers->irq.clear = VSI_IRQ_TIMER;
	config->registers->irq.enable = VSI_IRQ_TIMER;
	config->registers->regs[0] = 0U;
	config->irq_configure(dev);
	return 0;
}

#define VSI_I2S_INIT(instance)                                                                    \
	static void vsi_i2s_irq_configure_##instance(const struct device *dev)                     \
	{                                                                                           \
		ARG_UNUSED(dev);                                                                      \
		IRQ_CONNECT(DT_INST_IRQN(instance),                                                    \
			    DT_INST_IRQ(instance, priority),                                          \
			    vsi_i2s_isr,                                                               \
			    DEVICE_DT_INST_GET(instance),                                             \
			    0);                                                                        \
		irq_enable(DT_INST_IRQN(instance));                                                  \
	}                                                                                           \
	static struct vsi_i2s_data vsi_i2s_data_##instance;                                        \
	static const struct vsi_i2s_config vsi_i2s_config_##instance = {                            \
		.registers = (struct vsi_registers *)DT_INST_REG_ADDR(instance),                     \
		.source_file = DT_INST_PROP(instance, source_file),                                   \
		.irq_configure = vsi_i2s_irq_configure_##instance,                                    \
	};                                                                                          \
	DEVICE_DT_INST_DEFINE(instance,                                                             \
			      vsi_i2s_init,                                                         \
			      NULL,                                                                 \
			      &vsi_i2s_data_##instance,                                             \
			      &vsi_i2s_config_##instance,                                           \
			      POST_KERNEL,                                                          \
			      CONFIG_I2S_INIT_PRIORITY,                                             \
			      &vsi_i2s_driver_api);

DT_INST_FOREACH_STATUS_OKAY(VSI_I2S_INIT)
