.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_console:

=========
 Console
=========

TODO: description...



Using Console
==============

TODO: example...

.. code-block:: c

    /* TODO: exemple */
    void my_function(void)
    {
    }


.. note:: return 
    All the functions in this section will return one of the error codes defined in :c:type:`enum thinkos_err`.

-------------------------------------------------------------------------------


Console Functions
=================

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_console_read thinkos_console_timedread 
        thinkos_console_write

..
		thinkos_console_is_connected thinkos_console_close
        thinkos_console_drain thinkos_console_io_break
        thinkos_console_raw_mode thinkos_console_rd_nonblock
        thinkos_console_wr_nonblock

