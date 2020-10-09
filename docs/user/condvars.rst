.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_condvars:

=======================
 Conditional Variables
=======================

Condition variables are variables that represent certain conditions and can 
only be used in monitors. Associated with each condition variable, there is 
a queue of threads and two operations: condition signal and condition wait. 
When a thread calls condition wait, the caller is put into the queue of that 
condition variable. When a thread calls condition signal, if there are threads 
waiting in that condition variable's queue, one of them is released. 
Otherwise, the condition signal is lost.


Using Conditional Variables
===========================

Monitors
--------

Monitors are a kind of design pattern that produces reliable synchronization mechanism among threads avoiding race conditions by design.

This example shows a tipical monitor design.

The following is a typical use of a conditional variable within a monitor. 
The variable **cond** represents a condition that needs to be satisfied for 
the thread do gain access to the monitor, **lock** is a mutex lock for 
locking the monitor and **condvar** is a condition variable.

.. code-block:: c

    volatile bool cond;
    int lock;
    int condvar;

    void monitor(void) 
    {
        thinkos_mutex_lock(lock);

        while (cond != true) 
            thinkos_cond_wait(condvar, lock);


        /* do something ...* /

        thinkos_mutex_unlock(lock);
    }


    void process(void) 
    {
        /* do something ...* /

        /* change the condition ...* /
        cond = false;
        thinkos_cond_signal(condvar);

        /* do something else ...* /
        cond = false;
        thinkos_cond_signal(condvar);
    }


The first action upon entering is to call :c:func:`thinkos_mutex_lock` to locks the monitor, ensuring the atomic access to the condition **cond** . 

The monitor waits for the condition **cond** to be satisfied by calling :c:func:`thinkos_cond_wait`.


.. note:: return 
    All the functions in this section will return one of the error codes defined in :c:type:`enum thinkos_err`.

-------------------------------------------------------------------------------


Conditional Variables Functions
===============================

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_cond_alloc thinkos_cond_free 
        thinkos_cond_wait thinkos_cond_timedwait
        thinkos_cond_signal thinkos_cond_broadcast

