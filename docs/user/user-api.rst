.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_api:

==================
 ThinkOS User API 
==================

The user-space API manual gathers together documents describing aspects of
the ThinkOS_ interface as seen by application developers.

------------


Constants
=========


Error Codes
-----------

Most of the application level calls in ThinkOS_ return one of the values listed below.

.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_err, thinkos_obj_kind

-------------------------------------------------------------------------------


Kernel Objcets
==============

All kernel internal datastructures are referenced by object identifiers 
(handlers). Each object can be of the kinds (class) definded by 
:c:type:`enum thinkos_obj_kind`.

Most commonly these objects are allocated by using 
:c:func:`thinkos_obj_alloc` and freed by :c:func:`thinkos_obj_free`.


.. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_obj_alloc thinkos_obj_free

-------------------------------------------------------------------------------


Miscelaneous
============

The rest...

..
    .. kernel-doc:: sdk/include/thinkos.h
    :functions: thinkos_clocks thinkos_udelay_factor thinkos_abort 
        thinkos_critical_enter thinkos_critical_exit thinkos_escalate

-------------------------------------------------------------------------------


-------------------------------------------------------------------------------


Header Files
============


