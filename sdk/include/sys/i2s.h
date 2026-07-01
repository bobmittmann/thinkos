/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file sys/i2s.h
 * @brief YARD-ICE 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __SYS_I2S_H__
#define __SYS_I2S_H__

#include <stdint.h>
#include <stdbool.h>

#define I2S_RX_EN (1 << 0)
#define I2S_TX_EN (1 << 1)
#define I2S_MCK_EN (1 << 2)

#define I2S_16BITS (0 << 3)
#define I2S_24BITS (1 << 3)
#define I2S_32BITS (2 << 3)


/* driver statistics */
struct i2s_stats {
	uint32_t rx_cnt;
	uint32_t tx_cnt;
	uint32_t err_cnt;
};

struct i2s_dev;

enum {
	I2S_IOCTL_ENABLE = 0,
	I2S_IOCTL_DISABLE,
	I2S_IOCTL_DRAIN,
	I2S_IOCTL_RESET,
	I2S_IOCTL_STATS_GET,
	I2S_IOCTL_CONF_SET,
	I2S_IOCTL_CONF_GET
};

struct i2s_op {
	int (* close)(void *);
	int (* ioctl)(void *, int, uintptr_t, uintptr_t);
	int (* enable)(void *);
	int (* setbuf)(void *, int16_t *, int16_t *, size_t);
	int16_t * (* getbuf)(void *);
	uint32_t (* get_dma_tx_cnt)(void *);
	uint32_t (* get_dma_rx_cnt)(void *);
};

struct i2s_dev {
	void * drv;
	const struct i2s_op * op;
};

static inline int i2s_close(struct i2s_dev * dev){
	return dev->op->close(dev->drv);
}

static inline int i2s_ioctl(struct i2s_dev * dev,
							   int opt, uintptr_t arg1, uintptr_t arg2) {
	return dev->op->ioctl(dev->drv, opt, arg1, arg2);
}

static inline int i2s_reset(struct i2s_dev * dev) {
	return dev->op->ioctl(dev->drv, I2S_IOCTL_RESET, 0, 0);
}

static inline int i2s_stats_get(struct i2s_dev * dev,
								  struct i2s_stats * stats) {
	return dev->op->ioctl(dev->drv, I2S_IOCTL_STATS_GET, 
						  (uintptr_t)stats, 0);
}

static inline int i2s_enable(struct i2s_dev * dev) {
	return dev->op->ioctl(dev->drv, I2S_IOCTL_ENABLE, 1, 0);
}

static inline int i2s_disable(struct i2s_dev * dev) {
	return dev->op->ioctl(dev->drv, I2S_IOCTL_ENABLE, 0, 0);
}

static inline int i2s_setbuf(struct i2s_dev * dev, 
							 int16_t * buf1, int16_t * buf2, unsigned int len)
{
	return dev->op->setbuf(dev->drv, buf1, buf2, len);
}

static inline int16_t * i2s_getbuf(struct i2s_dev * dev)
{
	return dev->op->getbuf(dev->drv);
}

static inline uint32_t i2s_get_dma_tx_cnt(struct i2s_dev * dev)
{
	return dev->op->get_dma_tx_cnt(dev->drv);
}

static inline uint32_t i2s_get_dma_rx_cnt(struct i2s_dev * dev)
{
	return dev->op->get_dma_rx_cnt(dev->drv);
}

#ifdef __cplusplus
extern "C" {
#endif

int i2s_close(struct i2s_dev * dev);

int i2s_ioctl(struct i2s_dev * dev, int opt, 
				 uintptr_t arg1, uintptr_t arg2);

int i2s_stats_get(struct i2s_dev * dev, struct i2s_stats * stats);

int i2s_enable(struct i2s_dev * dev);

int i2s_disable(struct i2s_dev * dev);


#ifdef __cplusplus
}
#endif

#endif /* __SYS_I2S_H__ */

