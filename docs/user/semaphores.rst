.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_semaphores:

============
 Semaphores
============

ThinkOS_ semaphores allow processes and threads to synchronize their actions.

A semaphore is an integer whose value is never allowed to fall below zero. 
Two operations can be performed on semaphores: increment the semaphore value
by one :c:func:`thinkos_sem_post`; and decrement the semaphore value by 
one :c:func:`thinkos_sem_wait`.
If the value of a semaphore is currently zero, then a :c:fun:`thinkos_sem_wait`
operation will block until the value becomes greater than zero. A semaphore 
should be allocated with :c:func:`thinkos_sem_alloc` which will return a 
descriptor to be used in subsequent calls to  :c:func:`thinkos_sem_post`, 
and :c:func:`thinkos_sem_wait`.

Using semaphores
================

In the example below, two treads are created: a producer and a consumer. The producer signals the semaphore periodically twice a second, whereas the consumer waits for the signal and prints a message when receives it.


.. code-block:: c

    /* Global semaphore descriptor */
    int my_sem;

    /* Producer thread */
    static int producer_task(void * arg)
    {
        for (;;) {
            /* Wait 500 milliseconds... */
            thinkos_sleep(500);
            /* Signal the semaphore. */
            thinkos_sem_post(my_sem);
        }
        return 0;
    }

    /* Consumer thread */
    static int consumer_task(void * arg)
    {
        for (;;) {
            /* Wait on the semaphore... */
            thinkos_sem_wait(my_sem);
            /* Do something... */
            printf("Signal received\n");
        }
        return 0;
    }

    /* Thread's stacks */
    static uint32_t consumer_stack[256];
    static uint32_t producer_stack[256];

    void my_test_init(void)
    {
        /* Allocate a semaphore with 0 initial count. */
        my_sem = thinkos_sem_alloc(0);
        /* Create a thread to signal the semaphore */
        thinkos_thread_create(producer_task, NULL, 
                              producer_stack, sizeof(producer_stack));
        /* Create a thread to wait on the semaphore */
        thinkos_thread_create(consumer_task, NULL, 
                              consumer_stack, sizeof(consumer_stack));
    }


Remeber to create the threads calling :c:func:`thinkos_thread_create`. 
Please refer to the :ref:`thinkos_usr_threads` section for more information.

.. note:: return 
    All the functions in this section will return one of the error codes
    defined in :c:type:`enum thinkos_err`.

-------------------------------------------------------------------------------


Semaphore Functions
===================

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_sem_alloc thinkos_sem_free thinkos_sem_init 
        thinkos_sem_wait thinkos_sem_timedwait thinkos_sem_post 
        thinkos_sem_post_i

