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

/* character encoding and baud rate */
struct i2s_config {
};

/* modem control bits */
struct i2s_control {
	uint8_t dtr :1;
	uint8_t rts :1;
	/* sending line break */
	uint8_t brk :1;
};

struct i2s_status {
	/* modem status bits */
	uint8_t dsr :1;
	uint8_t ri :1;
	uint8_t dcd :1;
	uint8_t cts :1;
	/* receiving line break */
	uint8_t brk :1;
};

struct i2s_error {
	uint8_t ovr :1;
	uint8_t par :1;
	uint8_t frm :1;
};

struct i2s_dev;

enum {
	I2S_IOCTL_ENABLE = 0,
	I2S_IOCTL_DISABLE,
	I2S_IOCTL_DRAIN,
	I2S_IOCTL_RESET,
	I2S_IOCTL_FLUSH,
	I2S_IOCTL_FLOWCTRL_SET,
	I2S_IOCTL_STATS_GET,
	I2S_IOCTL_DMA_PREPARE,
	I2S_IOCTL_CONF_SET,
	I2S_IOCTL_CONF_GET,
	I2S_IOCTL_RX_TRIG_SET
};

struct i2s_op {
	int (* send)(void *, const void *, unsigned int);
	int (* recv)(void *, void *, unsigned int, unsigned int);
	int (* drain)(void *);
	int (* close)(void *);
	int (* ioctl)(void *, int, uintptr_t, uintptr_t);
	int (* enable)(void *);
	int (* setbuf)(void *, int16_t *, int16_t *, unsigned int);
	int16_t * (* getbuf)(void *);
};

struct i2s_dev {
	void * drv;
	const struct i2s_op * op;
};

static inline int i2s_send(struct i2s_dev * dev, const void * buf,
							  unsigned int len) {
	return dev->op->send(dev->drv, buf, len);
}

static inline int i2s_recv(struct i2s_dev * dev, void * buf,
							  unsigned int len, unsigned int msec) {
	return dev->op->recv(dev->drv, buf, len, msec);
}

static inline int i2s_drain(struct i2s_dev * dev) {
	return dev->op->drain(dev->drv);
}

static inline int i2s_close(struct i2s_dev * dev){
	return dev->op->close(dev->drv);
}

static inline int i2s_ioctl(struct i2s_dev * dev,
							   int opt, uintptr_t arg1, uintptr_t arg2) {
	return dev->op->ioctl(dev->drv, opt, arg1, arg2);
}

#if 0
static inline int i2s_drain(struct i2s_dev * dev) {
	return dev->op->ioctl(dev->drv, I2S_IOCTL_DRAIN, 0);
}
#endif

static inline int i2s_flush(struct i2s_dev * dev) {
	return dev->op->ioctl(dev->drv, I2S_IOCTL_FLUSH, 0, 0);
}


static inline int i2s_reset(struct i2s_dev * dev) {
	return dev->op->ioctl(dev->drv, I2S_IOCTL_RESET, 0, 0);
}

static inline int i2s_stats_get(struct i2s_dev * dev,
								  struct i2s_stats * stats) {
	return dev->op->ioctl(dev->drv, I2S_IOCTL_STATS_GET, 
						  (uintptr_t)stats, 0);
}

static inline int i2s_rx_enable(struct i2s_dev * dev) {
	return dev->op->ioctl(dev->drv, I2S_IOCTL_ENABLE, 
						  I2S_RX_EN, 0);
}

static inline int i2s_rx_disable(struct i2s_dev * dev) {
	return dev->op->ioctl(dev->drv, I2S_IOCTL_DISABLE, 
						  I2S_RX_EN, 0);
}

static inline int i2s_flowctrl_set(struct i2s_dev * dev,
									  unsigned int flowctrl) {
	return dev->op->ioctl(dev->drv, I2S_IOCTL_FLOWCTRL_SET, 
						  flowctrl, 0);
}

static inline int i2s_config_get(struct i2s_dev * dev,
									struct i2s_config * cfg)
{
	return dev->op->ioctl(dev->drv, I2S_IOCTL_CONF_GET, 
						  (uintptr_t)cfg, 0);
}

static inline int i2s_config_set(struct i2s_dev * dev,
									const struct i2s_config * cfg)
{
	return dev->op->ioctl(dev->drv, I2S_IOCTL_CONF_SET, 
						  (uintptr_t)cfg, 0);
}

static inline int i2s_rx_trig_set(struct i2s_dev * dev, 
							  unsigned int lvl)
{
	return dev->op->ioctl(dev->drv, I2S_IOCTL_RX_TRIG_SET, lvl, 0);
}

static inline int i2s_dma_prepare(struct i2s_dev * dev, 
							  void * buf, unsigned int len)
{
	return dev->op->ioctl(dev->drv, I2S_IOCTL_DMA_PREPARE, 
						  (uintptr_t)buf, len);
}

static inline int i2s_enable(struct i2s_dev * dev) {
	return dev->op->enable(dev->drv);
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


#ifdef __cplusplus
extern "C" {
#endif

int i2s_send(struct i2s_dev * dev, const void * buf, 
				unsigned int len);

int i2s_recv(struct i2s_dev * dev, void * buf, 
				unsigned int len, unsigned int msec);

int i2s_drain(struct i2s_dev * dev);

int i2s_close(struct i2s_dev * dev);

int i2s_config_get(struct i2s_dev * dev, struct i2s_config * cfg);

int i2s_config_set(struct i2s_dev * dev, 
					  const struct i2s_config * cfg);

int i2s_ioctl(struct i2s_dev * dev, int opt, 
				 uintptr_t arg1, uintptr_t arg2);

int i2s_stats_get(struct i2s_dev * dev, struct i2s_stats * stats);

int i2s_disable(struct i2s_dev * dev);

int i2s_rx_disable(struct i2s_dev * dev);

int i2s_rx_enable(struct i2s_dev * dev);


struct file * i2s_fopen(struct i2s_dev * dev);

bool is_i2s(struct file * f); 

/*
int i2s_control_get(struct i2s_dev * dev, struct i2s_control * ctrl);

int i2s_status_set(struct i2s_dev * dev, struct i2s_status * stat);
*/

#ifdef __cplusplus
}
#endif

#endif /* __SYS_I2S_H__ */

