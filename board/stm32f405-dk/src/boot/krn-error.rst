.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_krn_flashdrv:

==================================
 Kernel Errors and Faults Handling
==================================

ThinkOS provides a mechanism to trap usage errors on system calls as well as
some other run-time problems and faults.


Error Detection
===============

Each time the kernel is invoked an error detection mechanism can be involved.
The mechanism of detection resides in different parts of the kernel:

* System call
* Scheduler 
* User Fault - (Thread mode exceptions)
* Kernel Fault - (Handler mode exceptions)



System Call Errors
==================

If enabled any one of the following flags may raise a system call error:

* HINKOS_ENABLE_ARG_CHECK
* THINKOS_ENABLE_DEADLOCK_CHECK
* THINKOS_ENABLE_SANITY_CHECK


Error Processing with Monitor
-----------------------------

A system call handler invoke thinkos_krn_syscall_err() to raise the error.

If the **monitor** is enabled then the syscall_err() function perform the following tasks:

1. Masks all interrupts in the NVIC 
2. Mask all the threads
3. Mark the current thread context to break
4. Set current thread as faulty
5. Suspend current thread
6. Signal the scheduler
7. Report a thread_fault to the **monitor**

This will result in a chain of events.

Since the SysTick interrupt has a higher priority than the system calls. A SysTick event should be prevented until the current thread context is discarded. This mean that the scheduler should run at the exception return. 


A call to monitor_signal_thread_fault() unleashes a chain of events:

1. SysTick periodic (timer) interrupts are disabled. Pending SysTick interrupts are always enabled. 
2. THREAD_FAULT and SOFTRST signals are added to the monitor event set (raised)
3. A MONITOR_WAKEUP programmed into the IDLE loop.

The function then returns to the system call handler, which will do the same. The **scheduler** is initiated as a late arrival interrupts. Remember that the scheduler was signaled inside the error processing function (Step 6).

At this point the scheduler sees the current thread as an invalid - out of range - thread index. This indicates that the current thread's state shouldn't be preserved. It should be discarded instead. 

FIXME: In reality it should be copied to the exception buffer...









Error Processing without Monitor
--------------------------------

If the **monitor** is not enabled it will issue a breakpoint instruction. Then the processing depends on the **debugger** is enabled or not. Or if an ICE emulator (hardware debugger) is attached. 

If the user application is handling the errors by evaluating the system call's return values... (FIXME: need an option for this I guess) then the system call just returns.



-------------------------------------------------------------------------------



