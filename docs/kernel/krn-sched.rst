.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_krn_sched:

==================
 Kernel Scheduler
==================

The scheduler is responsible to select the next thread to run. Also some support routines may optionally performed by the scheduler:
* stack bounds check
* thread context discarding



Real Time scheduler
===================

The real-time scheduler ....

State
-----

+--------------------------+--------+
| System                   |  Sched |
+--------+--------+--------+--------+
| break  | error  |  mask  | active |
+--------+--------+--------+--------+
|        |        | 0x7f   |   0x31 |
+--------+--------+--------+--------+

Normal
~~~~~~

Normal operation the mask field should be set to 0xff.

Normal::

	ands  r1, r0, r0, asr #9

    00000000 00000000 11111111 yyyyxxxx
    00000000 00000000 00000000 01111111 1 -C--
    00000000 00000000 00000000 0yyyxxxx

Error::

    1eeeeeee 00000000 11111111 0yyyxxxx
    11111111 10000000 00000000 01111111 1 NC--
    1eeeeeee 00000000 00000000 0yyyxxxx

Forced Idle wakeup. Scheduler set the MSB of the `active` field to force 
the carry status flag.

Idle Normal::

    00000000 00000000 11111111 00000000
    00000000 00000000 00000000 01111111 1 -CZ-
    00000000 00000000 00000000 00000000

Force Idle::

    00000000 00000000 00000000 0yyyxxxx
    00000000 00000000 00000000 00000000 0 -Z--
    00000000 00000000 00000000 00000000

Discard
~~~~~~~

Force a non carry to discard the current thread. 
Mask field set to 0xfe.
Break field set to 0.

The scheduler will pick the 
next thread to run as usual. I the current thread is on the ready queue
it will be selected to run. This provides a mechanism to cancel the current
thread. As well as to replace the current thread execution context. 

This mechanism is used to run applications.

Discard::

    00000000 00000000 11111110 0yyyxxxx
    00000000 00000000 00000000 01111111 0 ----
    00000000 00000000 00000000 00000000


Break
~~~~~
	
To force a break condition the `brk` field must have the MSB bit set.
Mask field set to 0xfe.
 
Break::

    1eeeeeee 00000000 11111110 0yyyxxxx
    11111111 10000000 00000000 01111111 0 N---
    1eeeeeee 00000000 00000000 0yyyxxxx



Monitor 
-------


Flash driver with no monitor direct API.






-------------------------------------------------------------------------------


