/* 
 * thinkos_api.h
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
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

#ifndef __THINKOS_API_H__
#define __THINKOS_API_H__

/**
 * SECTION:kernel
 * @short_description: kernel functions
 * @title: ThinkOS Kernel 
 * @section_id:
 * @see_also: 
 * @stability: Stable
 * @include: thinkos.h
 * @image: application.png
 *
 */

/** 
 * thinkos_init:
 * @opt: Thread ID
 *
 * Initializes the ThinkOS library.
 *
 * On return the current program execution thread turns into the first 
 * thread of the system.
 *
 * Returns: THINKOS_OK
 */
int thinkos_init(unsigned int opt);

/** 
 * thinkos_mpu_init:
 * @size: size of the protected block
 *
 * Initializes the Cortex-M MPU.
 *
 * Returns: THINKOS_OK
 */
void thinkos_mpu_init(unsigned int size);

/** 
 * thinkos_userland:
 *
 * Switch processor to user mode.
 *
 * Returns: THINKOS_OK
 */
void thinkos_userland(void);

/**
 * SECTION:threads
 * @short_description: handle threads 
 * @title: ThinkOS Threads
 * @section_id:
 * @see_also: 
 * @stability: Stable
 * @include: thinkos.h
 * @image: application.png
 *
 */

/** 
 * thinkos_thread_create:
 * @task_ptr: pointer to the thread's starting function
 * @task_arg: argument to the thread's starting function
 * @stack_ptr: pointer the base of the thread's stack
 * @opt: size of the stack 
 *
 * Here’s a simple example:
 * |[<!-- language="C" -->
 *  GtkWidget *dialog;
 *  GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
 *  dialog = gtk_dialog_new_with_buttons ("My dialog",
 *                                        main_app_window,
 *                                        flags,
 *                                        _("_OK"),
 *                                        GTK_RESPONSE_ACCEPT,
 *                                        _("_Cancel"),
 *                                        GTK_RESPONSE_REJECT,
 *                                        NULL);
 * ]|
 *
 * Returns: -1 if call is not implemented, 0 otherwise. 
 **/
int thinkos_thread_create(int (* task_ptr)(void *), 
						  void * task_arg, void * stack_ptr,
						  unsigned int opt);

#endif /* __THINKOS_API_H__ */

