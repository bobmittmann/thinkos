/* 
 * File:	 sys/flash_dev.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __SYS_FLASH_DEV_H__
#define __SYS_FLASH_DEV_H__

#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct flash_dev_seq {
	intptr_t (* start)(intptr_t status, void * priv);
	intptr_t (* status)(intptr_t status, void * priv);
	intptr_t (* stop)(intptr_t status, void * priv);
	intptr_t (* finish)(intptr_t status, void * priv);
};

struct flash_dev_ops {
	int (* write)(void * priv, off_t offs, const void * buf, size_t count);
	int (* read)(void * priv, off_t offs, void * buf, size_t count);
	int (* lock)(void * priv, off_t offs, size_t count);
	int (* unlock)(void * priv, off_t offs, size_t count);
#if 0
	struct {
		intptr_t (* init)(void * priv, off_t offs, size_t count);
		const struct flash_dev_seq * seq;
	} erase;
#else
	int (* erase)(void * priv, off_t offs, size_t count);
#endif
};

/* FLASH memory device object */
struct flash_dev {
	void * priv;
	const struct flash_dev_ops * op;
};

static inline int flash_dev_write(const struct flash_dev * dev, 
								  off_t off, const void * buf, size_t count)
{
	return dev->op->write(dev->priv, off, buf, count);
}

static inline int flash_dev_read(const struct flash_dev * dev, 
								  off_t off, void * buf, size_t count)
{
	return dev->op->read(dev->priv, off, buf, count);
}

static inline int flash_dev_lock(const struct flash_dev * dev, 
								  off_t off, size_t count)
{
	return dev->op->lock(dev->priv, off, count);
}

static inline int flash_dev_unlock(const struct flash_dev * dev, 
								  off_t off, size_t count)
{
	return dev->op->unlock(dev->priv, off, count);
}

#if 1
static inline int flash_dev_erase(const struct flash_dev * dev, 
								  off_t off, size_t count)
{
	return dev->op->erase(dev->priv, off, count);
}
#else
static inline uintptr_t flash_dev_erase_init(const struct flash_dev * dev, 
											 off_t off, size_t count)
{
	return dev->op->erase.init(dev->priv, off, count);
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SYS_FLASH_DEV_H__ */

