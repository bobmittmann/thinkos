.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_krn_flash:

=====================
 Kernel Flash Driver
=====================

TODO: description...

ThinkOS porvides a kernel level flash memory driver.
A corresponding user access API is provided as well.


Partitions
==========

The flash memory usually is segmented in different *Partitions* with a unique *tag* or label.

In order to access the partition an application should call 
:ref:`thinkos_flash_mem_open` to obtain a *key* (handler).



Monitor 
=======


Flash driver with no monitor direct API.

This is a test version. The monitor executes an auxiliary thread which
handles the flash operations and report back the result upon
completion.

This approach should reduce the complexity of the code.
There is no need for a dual API (user/kernel) for the flash.
All it's need is the user API.
This makes sense since when the monitor is loading a new firmware
as well as erasing the flash no application should be running.




Using Flash Memory
==================

-------------------------------------------------------------------------------


Flash Memory Monitor Functions
==============================

