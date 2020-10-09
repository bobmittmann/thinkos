.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_threads:

=========
 Threads
=========

ThinkOS_ is a preemptive time-sharing kernel. Each thread has its own stack and a timer. This timer is used to block the thread's execution for a certain period of time as in \lstinline{thinkos_sleep()}.

The ThinkOS_'s kernel holds minimum information about a thread, only what is neccessary for the proper system operation, thus reducing the memory footprint. Tha absolute bare minimum configuration only the thread's stack pointer is managed by the kernel.

Using threads
=============


Creating a thread
-----------------

The easiest way to create and run a thread is to call :c:func:`thinkos_thread_create`. Here is an example: 

.. code-block:: c

    static int my_task(void * arg)
    {
        for (;;) {
            /* do something ... */
             thinkos_sleep(50);
        }
        return 0;
    }

    static uint32_t my_stack[256]; /* 1KB of stack */

    void my_module_init(void)
    {
        thinkos_thread_create(my_task, NULL, my_stack, sizeof(my_stack));
    }


In this example we define a function to be the entry point of the thread, in 
this case :c:func:`my_task()`. We also define an array of 32bit words, 
amounting to 1KiB, to be used as stack for the thread ``my_stack``. 

We then call :c:func:`thinkos_thread_create` which will allocate and run 
the thread. The return of this call function is the `thread id` or a negative 
error code in case something goes wrong.


.. note:: return 
    All the functions in this section will return one of the error codes
    defined in :c:enum:`enum thinkos_err`.

-------------------------------------------------------------------------------



Thread Functions
================

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_thread_create thinkos_thread_create_inf 
        thinkos_thread_self thinkos_cancel thinkos_exit 
        thinkos_join thinkos_pause thinkos_resume
        thinkos_yield thinkos_thread_abort

