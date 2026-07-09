.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_krn_comm:

=====================
 Kernel COMM Driver
=====================

ThinkOS porvides a kernel level communication channel abstraction to implement
serial or USB communication.
A corresponding user access API is provided as well.


COMM Objects and descriptors
============================

The kernel allocate comm objects consisting of 2 waiting queues per channel. Each waiting queue is identified by an object *descriptor*. These descriptors can be identified by the macros:

    * THINK_OS_COMM_TX_DESC(N) - comm descriptor for transmitting 
        for the *N* channel
    * THINK_OS_COMM_RX_DESC(N) - comm descriptor for receiving for 
        the *N* channel

Kernel Level Initialization
---------------------------

User API
--------

The user should call :ref:`comm_open()` to prepare the channel for receiving transmitting or both.
A call to :ref:`comm_close()` should be performed to shutdown the channel.



Operation 
=========

There are 4 layers involved in a transaction:
1. **User level**: Holds an object descriptor and call the kernel through system calls.
2. **Kernel SVC level**: knows the comm_block and queue associated with the od.
3. **COMM Device Driver**: Abstracts the underlying hardware to accept generic calls to send and receive. Ex: USB_CDC_ACM
4. **Hardware Device Driver**: The driver for the communication controller itself.Example: the STM32_USB_FS driver.

* `**comm_send(od, buf, len, tmo_ms)**`: initiate a transmission. The following chain of actions is performed:

1. User: call to comm_send()
2. Kernel SVC: 
  * Locate the thinkos_comm block corresponding to the descriptor **od**.
  * Creates a transaction structure, containing the data pointer (buf) the 
  * amount to transfer(len), thread number, (thread_id) and the queue 
  number (xmt_wq).
  * Current thread is put to sleep.
  * Signal the comm device driver: TX_PEND along with the transaction information 
  * Signal the scheduler execution (`__krn_sched_deferr()`)
3. COMM Driver (USB_CDC): 
  * Receive the TX_PEND signal. 
  * Insert the transaction into a priority queue.
  * Check for any pending transaction if free enable the channel for transmission:
    this can be done by unmasking an interrupt (USB SOF interrupt for example). 
4. 
  * USB Hardware device driver
  * Initiate a transaction 



-------------------------------------------------------------------------------

