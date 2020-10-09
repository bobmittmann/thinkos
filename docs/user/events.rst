.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_events:

========
 Events 
========

TODO: description...

Event Sets
----------



Using Event Sets
================

TODO: example...

.. code-block:: c

    /* TODO: exemple */
    void my_function(void)
    {
    }


.. note:: return 
    All the functions in this section will return one of the error codes defined in :c:type:`enum thinkos_err`.

-------------------------------------------------------------------------------


Events Functions
================

Include events system calls ...

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_ev_alloc thinkos_ev_free thinkos_ev_wait
        thinkos_ev_timedwait thinkos_ev_raise thinkos_ev_raise_i
        thinkos_ev_mask thinkos_ev_clear

