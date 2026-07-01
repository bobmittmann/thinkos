.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_krn_error:

===========================
 Kernel Selfosted Debugger
===========================

ThinkOS provides a mechanism to debug applications using a high priority monitor and the ARMv-7 DbgMon interrupt. 


Error Detection
===============

Each time the kernel is invoked an error detection mechanism can be involved.
The mechanism of detection resides in different parts of the kernel:

* System call
* Scheduler 
* User Fault - (Thread mode exceptions)
* Kernel Fault - (Handler mode exceptions)



Monitor
==================

If enabled any one of the following flags may raise a system call error:

* HINKOS_ENABLE_ARG_CHECK
* THINKOS_ENABLE_DEADLOCK_CHECK
* THINKOS_ENABLE_SANITY_CHECK


Error Processing with Monitor
-----------------------------

A system call handler invoke thinkos_krn_syscall_err() to raise the error.




-------------------------------------------------------------------------------



