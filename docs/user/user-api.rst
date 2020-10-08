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


Conditional Variables
=====================


Conditional Variables Functions
-------------------------------

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_cond_alloc thinkos_cond_free 
        thinkos_cond_wait thinkos_cond_timedwait
        thinkos_cond_signal thinkos_cond_broadcast

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


