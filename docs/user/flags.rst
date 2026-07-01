.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_flags:

=======
 Flags
=======

A flag is the simplest synchronization mechanism posible...


Using Flags
===========

The flags can be given and taken by a thread. Only a single thread can hold a flag.


.. code-block:: c

	/* TODO: flag exemple */
    void my_function(void)
    {
    }


.. note:: return 
    All the functions in this section will return one of the error codes defined in :c:type:`enum thinkos_err`.

-------------------------------------------------------------------------------


Flags Functions
===============

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_flag_alloc thinkos_flag_free thinkos_flag_val
        thinkos_flag_set thinkos_flag_clr thinkos_flag_give thinkos_flag_take 
        thinkos_flag_timedtake thinkos_flag_give_i

