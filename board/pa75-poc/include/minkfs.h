/* 
 * Copyright(C) 2015 Robinson Mittmann. All Rights Reserved.
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
 * @file minkfs.h
 * @brief Simple Flash Filesystem
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#ifndef __MINKFS_H__
#define __MINKFS_H__

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sflash.h>

enum minkfs_err {
	MINK_OK        =  0,  /**< No error */
	MINK_EINVAL    = -3,  /**< Invalid argument */
	MINK_EFAULT    = -8,  /**< Bad address */
	MINK_ENOMEM    = -9,  /**< Resource pool exausted */
	MINK_EEXIST	   = -10  /**< File exists */
};

/* Mink flash filesystem (opaque structure) */
struct mink_fs;

/* Mink file (opaque structure) */
struct mink_file;

/* Mink directory (opaque structure) */
struct mink_dir;

#ifdef __cplusplus
extern "C" {
#endif

int mink_file_open(struct mink_fs * fs, struct mink_file * file,
		const char * path, unsigned int mode);

int mink_file_create(struct mink_fs * fs, struct mink_file * file,
		const char * path, unsigned int mode);

int mink_file_read(struct mink_file * file, void * buf, size_t count);

int mink_file_write(struct mink_file * file, const void * buf, size_t count);

int mink_file_seek(struct mink_file * file, off_t offset, int whence);

int mink_file_close(struct mink_file * file);

int mink_stat(struct mink_fs * fs, const char * path, struct stat * buf);

int mink_unlink(struct mink_fs * fs, const char * path);

int mink_dir_create(struct mink_fs * fs, const char * path, unsigned int mode);

int mink_dir_open(struct mink_fs * fs, struct mink_dir * dirp, const char * path);

int mink_dir_close(struct mink_dir * dirp);

int mink_dir_read(struct mink_dir * dirp, struct dirent * entry, unsigned int count);

int mink_fs_init(struct mink_fs * fs, struct sflash_dev * sf);

struct mink_fs * mink_fs_alloc(void);

struct mink_fs * mink_fs_get_instance(void);

int mink_fs_mount(struct mink_fs * fs);

int mink_fs_garbage_collect(struct mink_fs * fs);

int mink_fs_format(struct mink_fs * fs);


struct mink_file * mink_file_alloc(void);

void mink_file_free(struct mink_file * file);

struct mink_dir * mink_dir_alloc(void);

void mink_dir_free(struct mink_dir * dir);

#ifdef __cplusplus
}
#endif

#endif /* __MINKFS_H__ */

