.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_gates:

=======
 Gates
=======

TODO: description...


Using Gates
===========

TODO: example...

.. code-block:: c

    /* TODO: exemple */
    void my_function(void)
    {
    }


.. note:: return 
    All the functions in this section will return one of the error codes defined in :c:type:`enum thinkos_err`.

-------------------------------------------------------------------------------


Gates Functions
===============

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_gate_alloc thinkos_gate_free thinkos_gate_wait
        thinkos_gate_timedwait thinkos_gate_open thinkos_gate_close
        thinkos_gate_exit thinkos_gate_open_i


