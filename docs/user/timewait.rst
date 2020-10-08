.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_timewait:

===========
 Time Wait
===========

ThinkOS_ is a preemptive time-sharing kernel. Each thread has its own stack and a timer. This timer is used to block the thread's execution for a certain period of time as in :c:func:`thinkos_sleep`.


Going to Sleep
==============

Use the :c:func:`thinkos_sleep` function to wait for a period of time. The 
calling thread suspends its execution until at least the time specified as 
parameter has elapsed. The time period is specified in milliseconds.
The example that follows a LED will blink repeatedly 4 times per second:

.. code-block:: c

    int blink_task(void * arg)
    {
        for (;;) {
            led_on();           /* Turn LED on */
            thinkos_sleep(100); /* Wait for 100 milliseconds */
            led_off();          /* Turn LED off */
            thinkos_sleep(150); /* Wait for 150 milliseconds */
        }

        return 0;
    }



.. note:: return 
    All the functions in this section will return one of the error codes
    defined in :c:enum:`enum thinkos_err`.

-------------------------------------------------------------------------------



Time Wait Functions
===================

Include calls to wait for a period of time to elapse or wake-up at a preset time (alarm).

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_sleep thinkos_clock thinkos_alarm 

