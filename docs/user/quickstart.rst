.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_quickstart:

=============
 Quick Start
=============


This guide will help you to install the necessary tools to compile and run a ThinkOS demo application on a development board.

Windows Tools
=============

These are the basic steps to follow in order to install the tools: 


- Install GNU Toolchain for ARM Processros for Windows hosts.
- Install GNU Make for Windows.
- Install GNU CoreUtils for Windows.
- Install the Java JRE from Oracle.
- Install the Eclipse for C++ Developers IDE.
- Install the GNU ARM Eclipse Plug-ins.
- Install the C/C++ GDB Hardware Debugging Plug-in.
- Install the STM32 ST-LINK Utility from STMicroelectronics.


Detailed instructions for each one of the steps can be found in the next sections of this document. 


Demo Application
================

There are some demo applications using \ThinkOS in the source tree. This is a guide on how to compile and run ThinkOS programs in the STM32F3Discovery board. 


- Get the ThinkOS_ Source Code from GitHub.
- Import the Eclipse_ projects for the target board into your workspace.
- Compile the ThinkOS_ BootLoader project using Eclipse IDE_.
- Load the ThinkOS BootLoader on the board using the STM32 ST-INK Utility.
- Install the Virtual COM Driver on the PC to access the ThinkOS Debug-Monitor.
- Compile the SDK Libraries (sdklib) for the target board using Eclipse.
- Compile the demo application project using Eclipse.
- Run the application with GDB Remote Debugging.


ThinkOS Basics
==============

In this tutorial the most basic \ThinkOS functions are described with some code examples. This is a summary of what it will be covered in this section:


- How to create and run threads.
- Create, lock and unlock mutexes.
- Create, signal and wait on semaphores.
- Wait for time intervals (sleep).
- Create a new thread


Useful Web References
=====================


