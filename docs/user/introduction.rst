.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_intro:

==============
 Introduction
==============


This user guide provides an overview of the ThinkOS_ real-time operating system. The target audience for this guide are embedded software developers knowledgeable in **C** language. Notice that no mention of C++ is made in this text.

ThinkOS_ was devised specifically for the `Cortex-M` family of microprocessors 
from ARM_. These are all 32bits CPUs with mostly 16bits instruction set, which 
produces very compact code, hence suitable for deep embedded systems with 
stringent memory and other peripheral resources constraints. 
The kernel takes advantage of some unique features of this processor to enable 
for very low task switch latency.

At this moment ThinkOS_ supports the `Cortex-M3`, `Cortex-M4` and `Cortex-M4F` 
cores.

The popular `M0` family lacks some features needed for some operations in the kernel. Porting to these platforms is feasible but needs someone with time to do that. Any volunteer?



Why Another OS?
===============

The real answer for this question perhaps is simply: *Just because I can*. But as most people won't be satisfied with this shallow reply here are some justifications:

Why Cortex-M
------------

The ARM Cortex-M family of microprocessors is the standard 32 bits CPU for 
most of the microcontrollers in the market today.

Can it be ported to another processor?
--------------------------------------

In theory yes. But probably its not worth the effort...


Where do I can get ThinkOS from?
--------------------------------

* Repository: [\href{https://github.com/bobmittmann/thinkos}{https://github.com/bobmittmann/thinkos}]
* Git HTTPS: [\href{https://github.com/bobmittmann/thinkos.git}{https://github.com/bobmittmann/thinkos.git}]

How to use it?
--------------

How and what is need in order to compile ThinkOS_ ....

- Download the source code
- Install the Tools
- Configure / or chose and example
- Compile
- Load and run


Performance
-----------

The main bottleneck for performance is usually the scheduler. The ThinkOS running in a 120MHz Cortex-M3 (STM32F270) have a measured latency time of 0.5 microseconds.

There are some tests and calculations that need to be done to determine the latency of other elements of the system...


Source Code
-----------

One feature that distinguishes the \ThinkOS from most of other Operating Systems is the way the control structures for the several types of objects are held by kernel.

The usual approach for holding this information is to allocate a structure specific for a certain object and pass a pointer to the kernel. The kernel then stores this in a list or array. 

ThinkOS_ in another hand holds the core data structure in a *Structure of Arrays* (SOA) representation. The cost of this representation is the readability of the code, (it may look a little obscure at the beginning). 
The idea here is simplify the race condition avoidance in the kernel and speed-up the scheduler.


Coding Style
------------------------------------------------

\ThinkOS source uses the \href{https://www.kernel.org/doc/html/v4.10/process/coding-style.html}{Linux kernel coding style} and I encourage all developers to follow it. This is just little more than a matter of preference, please read the style guide to know the rational behind the rules before despising them :) ...


Compiling
==================

There are basically two ways of using \ThinkOS:

\paragraph{As a library}{ linked against your application. (Need a term to decribe this like embedded mode...). 
In this type of usage there is usually no bootloader, the application boots and configure itself. In this scenario the firmware is the application. The advatnages of this mode are:}

- Smallest footprint. The linker will strip away unused (unreferenced) portions of the kernel.
- Kernel calls and services are tailored for the application.
	
\paragraph{As part of the bootloader}{ separated from the application. This mode will be called "standalone" mode, or "bootloader" mode.
In this case the application will be a separated entity (file, binary) and run in a separated memory space. The bootloader will be residing on the platform at all times and will be responsible to initialize the platform and to load the application. It's possible then to upload upgrade and change the application without changing the bootlader and consequently the operating system. Advantages:}


- No need to configure the OS (need to elaborate...) to match the application. At least initially.
- In field firmware upgrades are safer as the bootloader is the responsible for the upgrade.
- The Debug Monitor features can be used for development.
- Kernel data and code memory are protected.


% System Calls
------------------------------------------------
Modules
==================
 Overview of the different modules.

- Kernel
- Debug Monitor
- GDB server
- Console



Threads
==================

\ThinkOS is a preemptive time-sharing kernel. Each thread has its own stack and a timer. This timer is used to block the thread's execution for a certain period of time as in \lstinline{thinkos_sleep()}.

Scheduler
==================
ThinkOS scheduler was designed to reduce the context switch time to a minimum. The most important piece on the Kernel is the Scheduler....

Time Sharing
==================
All threads with priority higher than 0 go into a round-robin time sharing scheduling policy, when this feature is enabled. Their execution time will be inversely proportional to their priority.

Interrupt Model
==================
There are two types of interrupt handlers with ThinkOS:

\paragraph{Implicit ISR}{Thread interrupt handlers that executes as normal threads. This is a simple call to \lstinline{thinkos_irq_wait(IRQ)} inside a regular task or function. The thread will block until this interrupt is raised. This mechanism provides a convenient and powerful tool to design drivers. Devices that benefit most of this type of ISR are simplex I/O channels like I2C or SPI, hardware accelerators like cryptographic ...}

\paragraph{Native ISR}{Normal interrupt handlers that shares a common stack. These handlers are subject to to some constraints regarding API calls. It is recommended to use this type of interrupt handler when low latency is a must. But bear in mind that if the handler is too complex the actual latency due to loading registers and housekeeping may be equivalent to a context switch in which case it will be equivalent of a threaded handler in terms of performance. Another case will be a high priority interrupt you want to make sure it will won't be preempted...}


Configuration is simply a matter of creating your custom configuration header file: \lstinline{config.h} and enabling or disabling the features the application requires.

The file \lstinline{include/thinkos_sys.h} contains the definition of the structures used by the OS. In this file there is a series of macros used to set default values for the configuration options.

It is not recommended to change the values directly in \lstinline{thinkos_sys.h}, create a \lstinline{config.h} file instead, and define the macro \lstinline{CONFIG_H} at the compiler's: \lstinline{-DCONFIG_H}.

Note: if you are using the \ThinkOS as part of the YARD-ICE there is a \lstinline{config.h} file located at the source root.


% ----------------------------------------------------------------------------

\section {Quick Start}

This guide will help you to install the necessary tools to compile and run a ThinkOS demo application on a development board.

Windows Tools
==================

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
==================
There are some demo applications using \ThinkOS in the source tree. This is a guide on how to compile and run ThinkOS programs in the STM32F3Discovery board. 



- Get the \ThinkOS Source Code from GitHub.
- Import the Eclipse projects for the target board into your workspace.
- Compile the \ThinkOS BootLoader project using Eclipse IDE.
- Load the \ThinkOS BootLoader on the board using the STM32 ST-LINK Utility.
- Install the Virtual COM Driver on the PC to access the ThinkOS Debug-Monitor.
- Compile the SDK Libraries (sdklib) for the target board using Eclipse.
- Compile the demo application project using Eclipse.
- Run the application with GDB Remote Debugging.




ThinkOS Basics
==================

\lstset {language=C99}

In this tutorial the most basic \ThinkOS functions are described with some code examples. This is a summary of what it will be covered in this section:


- How to create and run threads.
- Create, lock and unlock mutexes.
- Create, signal and wait on semaphores.
- Wait for time intervals (sleep).
- Create a new thread


Compiling the Code
------------------------------------------------

In order to compile the code in a Windows machine you must first install these tools:

- GNU Toolchain for ARM Processors for Windows hosts.
- GNU Make for Windows.


Compiling Using the Command Shell
------------------------------------------------

You can use the windows Command Prompt or a MinGW/MSYS terminal to compile the code.
Open a windows shell: Start->All Programs->Accessories->Command Prompt
Change to the directory  in the ThinkOS source tree.
Type: make

