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
 * @file dirent.h
 * @brief YARD-ICE libc
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __DIRENT_H__
#define __DIRENT_H__

#include <sys/types.h>
#define __need_NULL
#include <stddef.h>

#define MAXNAMLEN 53

struct dirent {
	ino_t          d_ino;       /* inode number */
    off_t          d_off;       /* offset to the next dirent */
    unsigned short d_reclen;    /* length of this record */
    char           d_name[MAXNAMLEN - 1];  /* filename */
};

struct dir;

typedef struct dir DIR;

#ifdef __cplusplus
extern "C" {
#endif

DIR * opendir(const char * name);

struct dirent * readdir(DIR * dirp);

void rewinddir(DIR * dirp);

int closedir(DIR * dirp);

#endif /* __DIRENT_H__ */
