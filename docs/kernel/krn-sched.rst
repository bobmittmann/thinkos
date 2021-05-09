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


Scheduler State
---------------

+-------+-------+-------+-------+
|   err |   brk |   svc |   act |
+-------+-------+-------+-------+
|  0x00 |  0x00 |  0x00 |  0x00 |
+-------+-------+-------+-------+

 * *act* - active thread
 * *svc* - pending service
 * *brk* - thread active on break
 * *err* - error code

+------------+-------+-------+-------+-------+
|            |   err |   brk |   svc |   act |
+------------+-------+-------+-------+-------+
| Normal     |  == 0 |  == 0 |  == 0 |  == 0 |
+------------+-------+-------+-------+-------+
| Normal     |  == 0 |  == 0 |  == 0 |  >= 1 |
+------------+-------+-------+-------+-------+
| Error      |  >= 1 |     ? |     ? |    ?  |
+------------+-------+-------+-------+-------+
| Debugger   |  == 0 |  >= 1 |     ? |    ?  |
+------------+-------+-------+-------+-------+
| Reserved   |  == 0 |  == 0 |  >= 1 |    ?  |
+------------+-------+-------+-------+-------+


Case 1 - normal
---------------

The scheduler was activated and a thread is running.

act != 0

Case 2 - discard
----------------

act == 0

The scheduler was activated but active thread is set to NULL. 
 1. Check exec return
   a. If PSPSEL == 1 discard
   b. If PSPSEL == 1 save Idle 
 2. run the scheduler ....

Only a system call can set the current thread to NULL.

Case 3 - paused
---------------

The kernel scheduler is disabled/paused. Active thread is NULL (zero) and the flag is not set. 
In this case simply schedule the Idle thread. No context is saved or restored. 


Case 4 - discard
----------------

If the control field is 0 then discard the active thread (don't save context) and move on.

Case 5 - error
--------------

if control > 0, this is a forced error. An error was detected by the monitor as a result of a syscall invalid parameter for example. 
A fault on a thread or a user code breakpoint would result in this state.

 - The active thread is moved to the fault field. 
 - The active is set to 0. This will effectivelly pause the scheduler. 
 - It shuld be reenabled by the monitor.
 - The monitor is signaled.

Case 6 - break
--------------





::
    hh >> 24


::
    ldr.n    r0, .L_thinkos_rt
    ldrd     r1, r2, [r0, #OFFSETOF_KRN_SCHED]
    asrs     r1, #24
    bls      .L_except




Alternative
===========



State
-----

+--------------------------+--------+
| System                   |  Sched |
+--------+--------+--------+--------+
| break  |   code |  mask  | active |
+--------+--------+--------+--------+
|        |        | 0x7f   |   0x31 |
+--------+--------+--------+--------+

::
    ldr.n    r0, .L_thinkos_rt
    ldrd     r1, r2, [r0, #OFFSETOF_KRN_SCHED]
    ands     r1, r1, r1, asr #9
    bcc      .L_entry_xcept /* C flag clear: exception */
    beq      .L_entry_from_idle /* Z flag set */

Normal
~~~~~~

Normal operation the `mask` field should be set to 0xff.
This will cause the carry flag to be set and the active thread id to
be the return of the **ands** operation.


Normal::
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

The scheduler will pick the next thread to run as usual. 
If the current thread is on the ready queue
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



Other 
~~~~~
	
Set the To force a break condition the `brk` field must have the MSB bit set.
Mask field set to 0xfe.
 
Break::

    1eeeeeee 00000000 11111110 0yyyxxxx
    11111111 10000000 00000000 01111111 0 N---
    1eeeeeee 00000000 00000000 0yyyxxxx



Monitor 
-------


Flash driver with no monitor direct API.






-------------------------------------------------------------------------------


