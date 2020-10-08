.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_interrupts:

============
 Interrupts 
============

TODO: description...

Interrupt Requests
------------------

Using Interrupts
=================

TODO: example...

.. code-block:: c

    /* TODO: exemple */
    void my_function(void)
    {
    }


.. note:: return 
    All the functions in this section will return one of the error codes defined in :c:type:`enum thinkos_err`.

-------------------------------------------------------------------------------


Interrupts Functions
====================

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_irq_wait thinkos_irq_timedwait thinkos_irq_register


