/* 
 * thinkos_api.c
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
 * but WITHOUT ANY WARRANTY; without flagen the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#include <thinkos.h>

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
int 
thinkos_thread_create(int (* task_ptr)(void *), 
						  void * task_arg, void * stack_ptr,
						  unsigned int opt)
{
	return 0;
}

