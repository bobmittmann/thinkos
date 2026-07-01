.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_flashmem:

==============
 Flash Memory
==============

TODO: description...

ThinkOS porvides a kernel level flash memory access API allowing
safe operations to be performed by the applicationm.


Partitions
----------

The flash memory usually is segmented in different *Partitions* with a unique *tag* or label.

In order to access the partition an application should call 
:ref:`thinkos_flash_mem_open` to obtain a *key* (handler).


Using Flash Memory
==================

TODO: example...

.. code-block:: c

    /* TODO: exemple */
    void my_function(void)
    {
    }


.. note:: return 
    All the functions in this section will return one of the error codes defined in :c:type:`enum thinkos_err`.

-------------------------------------------------------------------------------


Flash Memory Functions
======================

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_flash_mem_open thinkos_flash_mem_close
        thinkos_flash_mem_open thinkos_flash_mem_read thinkos_flash_mem_write
        thinkos_flash_mem_erase thinkos_flash_mem_lock thinkos_flash_mem_unlock

