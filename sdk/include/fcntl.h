/* 
 * Copyright(c) 2004-2012 BORESTE (www.boreste.com). All Rights Reserved.
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
 * @file fcntl.h
 * @brief YARD-ICE libc
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __FCNTL_H__
#define __FCNTL_H__

#define	_FOPEN		(-1)	/* from sys/file.h, kernel use only */
#define	_FREAD		0x0001	/* read enabled */
#define	_FWRITE		0x0002	/* write enabled */
#define	_FNDELAY	0x0800	/* non blocking I/O (4.2 style) */
#define	_FAPPEND	0x0400	/* append (writes guaranteed at the end) */
#define	_FMARK		0x0010	/* internal; mark during gc() */
#define	_FDEFER		0x0020	/* internal; defer for next gc pass */
#define	_FASYNC		0x2000	/* signal pgrp when data ready */
#define	_FCREAT		0x0040	/* open with file create */
#define	_FTRUNC		0x0200	/* open with truncation */
#define	_FEXCL		0x0080	/* error on open if file exists */
#define	_FNBIO	    _FNONBLOCK	/* non blocking I/O (sys5 style) */
#define	_FSYNC		0x1000	/* do all writes synchronously */
#define	_FNONBLOCK	0x0800	/* non blocking I/O (POSIX style) */
#define	_FNOCTTY	0x0100	/* don't assign a ctty on this open */

#define	O_ACCMODE	(O_RDONLY|O_WRONLY|O_RDWR)

/*
 * Flag values for open(2) and fcntl(2)
 * The kernel adds 1 to the open modes to turn it into some
 * combination of FREAD and FWRITE.
 */
#define	O_RDONLY	0		/* +1 == FREAD */
#define	O_WRONLY	1		/* +1 == FWRITE */
#define	O_RDWR		2		/* +1 == FREAD|FWRITE */
#define	O_APPEND	_FAPPEND
#define	O_CREAT		_FCREAT
#define	O_TRUNC		_FTRUNC
#define	O_EXCL		_FEXCL
/*	O_SYNC		_FSYNC		not posix, defined below */
/*	O_NDELAY	_FNDELAY 	set in include/fcntl.h */
/*	O_NDELAY	_FNBIO 		set in 5include/fcntl.h */
#define	O_NONBLOCK	_FNONBLOCK
#define	O_NOCTTY	_FNOCTTY


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __FCNTL_H__ */

