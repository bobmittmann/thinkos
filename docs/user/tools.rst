.. -*- coding: utf-8; mode: rst -*-
.. include:: refs.txt

.. _thinkos_usr_tools:

==============
 Tools 
==============


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


MSYS-2
======

MSYS2 is software distribution and a building platform for Windows. It provides a Unix-like environment, a command-line interface and a software repository making it easier to install, use, build and port software on Windows.\\

The official website is:

MSYS2 http://www.msys2.org Website

Useful information can be found at the project's wiki pages at: 

https://github.com/msys2/msys2/wiki/MSYS2-introduction



GNU Toolchain for ARM Processors
================================

The GNU toolchain is a broad collection of programming tools developed by the GNU Project. These tools form a toolchain (a suite of tools used in a serial manner) used for developing software applications and operating systems.



If you are a windows user, you can download and install the GNU Toolchain for ARM Processors from the site: https://launchpad.net/gcc-arm-embedded. At the time of writing this document the latest version known to work with YARD-ICE was 4.9-2015-q3, which can be downloaded directly from:


GNU Arm Embedded Toolchain at Launchpad:
https://launchpad.net/gcc-arm-embedded


Arm Embedded Toolchain:
 https://developer.arm.com/open-source/gnu-toolchain/gnu-rm

Downloads:
https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads


Run the downloaded file \textit{gcc-arm-none-eabi-6-2017-q2-update-win32-sha2.exe}, or newer, to start the installation.
In the next steps use the default values suggested by the wizard installer.
In the last screen, after the files were copied, select the "Add path to environment variable" option.

Alternatively a compressed (.zip) file can be downloaded (\textit{gcc-arm-none-eabi-6-2017-q2-update-win32.zip}) and extracted in a directory of choice...

GNU Make
------------------------------------------------

GNU Core Utils for Windows
------------------------------------------------

Python
======

Python is an interpreted, high-level and general-purpose programming language.


Julia
=====

Julia is a high-level, high-performance, dynamic programming language. While it is a general purpose language and can be used to write any application, many of its features are well-suited for numerical analysis and computational science.

Wbsite: https://julialang.org/


Eclipse IDE
===========

Eclipse is an integrated development environment used in computer programming. It contains a base workspace and an extensible plug-in system for customizing the environment.


Eclipse CDT
-----------

The CDT Project provides a fully functional C and C++ Integrated Development Environment based on the Eclipse platform.


GNU ARM Eclipse Plug-Ins
------------------------

Quick info on GNU ARM Eclipse Plug-Ins:

* Location: http://gnuarmeclipse.sourceforge.net/updates
* Website: http://gnuarmeclipse.livius.net/blog


GNU MCU Eclipse plug-ins
------------------------

These plug-ins provide Eclipse CDT (C/C++ Development Tooling) extensions for GNU embedded toolchains like the GNU Tools for ARM Embedded Processors used as reference in this document.


* Eclipse update site: http://gnu-mcu-eclipse.netlify.com/v4-neon-updates
* Website:https://gnu-mcu-eclipse.github.io


Tera-Term
=========

Tera Term is an open-source, free, software implemented, terminal emulator program. It emulates different types of computer terminals, from DEC VT100 to DEC VT382. It supports telnet, SSH 1 & 2 and serial port connections. It also has a built-in macro scripting language and a few other useful plugins.


Git - Revision Control
======================






Git Extensions
==============

From the project's website: `Git Extensions is a toolkit aimed at making working with Git under Windows more intuitive (note that Git Extensions is also available on Linux and Macintosh OS X using Mono). The shell extension will integrate in Windows Explorer and presents a context menu on files and directories.`

Website: http://git-extensions-documentation.readthedocs.io/en/latest/git_extensions.html


YARD-ICE
========

The YARD-ICE project is a community based, open source hardware and software platform for a standalone remote debugger. The embedded platform uses \ThinkOS as bootloader and real-time operating system.


- YARD-ICE Project Location: https://github.com/bobmittmann/yard-ice
- Latest Release: https://github.com/bobmittmann/yard-ice/archive/0.24.zip
- Git Repository (HTTPS): https://github.com/bobmittmann/yard-ice.git
- Git Repository (SSH): git@github.com:bobmittmann/yard-ice.git




Gnu Tools for Window
=====================

More information on the GNU Make and CoreUtils for Windows package can be found at:

- GNU Make:  http://gnuwin32.sourceforge.net/packages/make.html
- Coreutils: http://gnuwin32.sourceforge.net/packages/coreutils.htm


Useful Web References
=====================

ThinkOS on Windows Host
=======================

- this document is a guide on how to set-up the tools and environment to develop ThinkOS applications using a Windows computer as host.



