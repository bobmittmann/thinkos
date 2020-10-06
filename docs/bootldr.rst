====================
 ThinkOS Bootloader
====================

The Bootloader module provides an configurable and extensible framework to
allow developers to design custom bootloaders with integrated firmware 
management, and debug facility.


kernel-render:: DOT
   :alt: foobar digraph
   :caption: Embedded **DOT** (Graphviz) code

   digraph foo {
    "bar" -> "baz";
   }


Bootloader Console
==================

USB
---

Serial
------


Bootloader Monitor
==================


Console Debugger
================


Embedded GDB Server
===================


Configuration options
=====================


=========== ============== ============= ============
 EXC_RETURN    Return to   Return stack   Frame type
=========== ============== ============= ============
 0xFFFFFFF1  Handler mode          Main        Basic
 0xFFFFFFF9   Thread mode          Main        Basic
 0xFFFFFFFD   Thread mode       Process        Basic
=========== ============== ============= ============

