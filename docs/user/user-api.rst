.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_api:

==================
 ThinkOS User API 
==================

The user-space API manual gathers together documents describing aspects of
the ThinkOS_ interface as seen by application developers.

------------


Constants
=========


Error Codes
-----------

Most of the application level calls in ThinkOS_ return one of the values listed below.

.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_err, thinkos_obj_kind

-------------------------------------------------------------------------------


Kernel Objcets
==============

All kernel internal datastructures are referenced by object identifiers 
(handlers). Each object can be of the kinds (class) definded by 
:c:type:`enum thinkos_obj_kind`.

Most commonly these objects are allocated by using 
:c:func:`thinkos_obj_alloc` and freed by :c:func:`thinkos_obj_free`.


.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_obj_alloc thinkos_obj_free

-------------------------------------------------------------------------------


Threads
=======

Threads are nice to have.

.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_thread_create thinkos_thread_create_inf 
		thinkos_thread_self thinkos_cancel thinkos_exit 
		thinkos_join thinkos_pause thinkos_resume
		thinkos_yield thinkos_thread_abort

.. note:: return 
	All the functions in this section will return one of the error codes
	defined in :c:type:`enum thinkos_err`.

-------------------------------------------------------------------------------


Mutexes
=======

A mutex is a MUTual EXclusion device, and is useful for protecting shared data 
structures from concurrent modifications, and implementing critical sections 
and monitors.

A mutex has two possible states: unlocked (not owned by any thread), and 
locked (owned by one thread). A mutex can never be owned by two different threads 
simultaneously. A thread attempting to lock a mutex that is already locked by 
another thread is suspended until the owning thread unlocks the mutex first.

Using MUTEXes
-------------

A call to :c:func:`thinkos_mutex_lock` locks the given mutex. If the mutex is 
currently unlocked, it becomes locked and owned by the calling thread, 
and :c:func:`thinkos_mutex_lock` returns immediately. If the mutex is already 
locked by another thread, :c:func:`thinkos_mutex_lock` suspends the calling 
thread until the mutex is unlocked by calling :c:func:`thinkos_mutex_unlock`. 
A mutex should be allocated with :c:func:`thinkos_mutex_alloc` which will 
return a descriptor to be used in subsequent mutex calls.

In the following example the serial port is a resource that needs to be 
protected to ensure the packets are transmitted in full.

.. code-block:: c

	/* Link Layer private data */
	static struct {
		uint8_t addr; /* this panel address */
		struct serial_dev * serial; /* serial device driver */
		int mutex; /* protect the link layer send call */
	} rs485;

	void netlnk_send(unsigned int daddr, void * data, unsigned int len)
	{
		struct lnkhdr hdr;

		/* lock the mutex, blocking the access to the serial port... */
		thinkos_mutex_lock(rs485.mutex);
		/* prepare header */
		hdr.sync = PKT_SYNC;
		hdr.daddr = daddr;
		hdr.saddr = rs485.addr;
		hdr.datalen = len;
		/* send header */
		serial_send(rs485.serial, &hdr, sizeof(struct lnkhdr));
		/* send payload */
		serial_send(rs485.serial, data, len);
		/* unlock mutex, other thread can use the serial port now. */
		thinkos_mutex_unlock(rs485.mutex);
	}

	void netlnk_init(unsigned int addr)
	{
		/* Open the serial port */
		rs485.serial = stm32f_uart1_serial_init(9600, SERIAL_8N1);
		/* Set the local address */
		rs485.addr = addr;
		/* Allocate a mutex */
		rs485.mutex = thinkos_mutex_alloc();
	}


.. note:: return 
	All the functions in this section will return one of the error codes defined in :c:type:`enum thinkos_err`.


Mutex Functions
---------------

.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_mutex_alloc thinkos_mutex_free thinkos_mutex_lock
		thinkos_mutex_trylock thinkos_mutex_timedlock
		thinkos_mutex_unlock

-------------------------------------------------------------------------------


Conditional Variables
=====================


Conditional Variables Functions
-------------------------------

.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_cond_alloc thinkos_cond_free 
		thinkos_cond_wait thinkos_cond_timedwait
		thinkos_cond_signal thinkos_cond_broadcast

-------------------------------------------------------------------------------


Semaphores
==========


Classic semaphores...

.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_sem_alloc thinkos_sem_free thinkos_sem_init 
		thinkos_sem_wait thinkos_sem_timedwait thinkos_sem_post 
		thinkos_sem_post_i

-------------------------------------------------------------------------------


Flags
=====

Include flags system calls ...

.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_flag_alloc thinkos_flag_free thinkos_flag_val
		thinkos_flag_set thinkos_flag_clr thinkos_flag_give thinkos_flag_take 
		thinkos_flag_timedtake thinkos_flag_give_i

-------------------------------------------------------------------------------


Gates
=====

Include gates system calls ...

Gates Functions
---------------

.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_gate_alloc thinkos_gate_free thinkos_gate_wait
		thinkos_gate_timedwait thinkos_gate_open thinkos_gate_close
		thinkos_gate_exit thinkos_gate_open_i


-------------------------------------------------------------------------------


Events
======

Include events system calls ...

.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_ev_alloc thinkos_ev_free thinkos_ev_wait
		thinkos_ev_timedwait thinkos_ev_raise thinkos_ev_raise_i
		thinkos_ev_mask thinkos_ev_clear

-------------------------------------------------------------------------------


Time Wait
=========

Include calls to wait for time to elapse...

.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_sleep thinkos_clock thinkos_alarm 

-------------------------------------------------------------------------------


Interrupt Requests
==================

Include flags system calls ...

.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_irq_wait thinkos_irq_timedwait thinkos_irq_register


-------------------------------------------------------------------------------


Flash Memory
============

ThinkOS porvides a kernel level flash memory access API allowing
safe operations to be performed by the applicationm.


Partitions
----------

The flash memory usually is segmented in different *Partitions* with a unique *tag* or label.

In order to access the partition an application should call 
:ref:`thinkos_flash_mem_open` to obtain a *key* (handler).


Flash Memory Functions
----------------------

.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_flash_mem_open thinkos_flash_mem_close
		thinkos_flash_mem_open thinkos_flash_mem_read thinkos_flash_mem_write
		thinkos_flash_mem_erase thinkos_flash_mem_lock thinkos_flash_mem_unlock

-------------------------------------------------------------------------------


Miscelaneous
============

The rest...

..
	.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_clocks thinkos_udelay_factor thinkos_abort 
		thinkos_critical_enter thinkos_critical_exit thinkos_escalate

-------------------------------------------------------------------------------


Trace
=====

..
	.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_trace thinkos_trace_open thinkos_trace_close 
	thinkos_trace_read thinkos_trace_flush thinkos_trace_getfirst 
	thinkos_trace_getnext


-------------------------------------------------------------------------------


Console
=======

..
	.. kernel-doc:: sdk/include/thinkos.h
	:functions: thinkos_console_read thinkos_console_timedread 
		thinkos_console_write
		thinkos_console_is_connected thinkos_console_close
		thinkos_console_drain thinkos_console_io_break
		thinkos_console_raw_mode thinkos_console_rd_nonblock
		thinkos_console_wr_nonblock

-------------------------------------------------------------------------------


Header Files
============


