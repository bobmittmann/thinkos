.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_mutexes:

=========
 MUTEXes
=========

A mutex is a MUTual EXclusion device, and is useful for protecting shared data 
structures from concurrent modifications, and implementing critical sections 
and monitors.

A mutex has two possible states: unlocked (not owned by any thread), and 
locked (owned by one thread). A mutex can never be owned by two different threads 
simultaneously. A thread attempting to lock a mutex that is already locked by 
another thread is suspended until the owning thread unlocks the mutex first.


Using MUTEXes
=============

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

-------------------------------------------------------------------------------


Mutex Functions
===============

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_mutex_alloc thinkos_mutex_free thinkos_mutex_lock
        thinkos_mutex_trylock thinkos_mutex_timedlock
        thinkos_mutex_unlock

