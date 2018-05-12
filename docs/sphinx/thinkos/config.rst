
Configuration
=============

.. default-domain:: c

.. describe:: #ifndef __THINKOS_KERNEL__
	#include <thinkos/kernel.h>

Configuration Options
---------------------

The ThinkOS kernel and all other libraries distributed are configured by selecting...


	.. list-table::
		:widths: 50 10 10 100
		:header-rows: 1
		:align: left 

		*	- Macro
			- Default Value
			- Valid Values
			- Description

		*	- .. c:macro:: THINKOS_THREADS_MAX
			- 8
			- 1..32
			- Set the maximum number of concurrent threads

		*	- .. c:macro:: THINKOS_ENABLE_THREAD_ALLOC
			- 1
			- 0 | 1
			- Enable/disable dynamic thread allocation

		*	- .. c:macro:: THINKOS_ENABLE_THREAD_INFO
			- 1
			- 0 | 1
			- Enable/disable thread info

		*	- .. c:macro:: THINKOS_ENABLE_THREAD_STAT
			- 1
			- 0 | 1
			- Enable/disable thread status register

		*	- .. c:macro:: THINKOS_ENABLE_JOIN
			- 1
			- 0 | 1
			- Enable/disable wait on thread completion

		*	- .. c:macro:: THINKOS_ENABLE_PAUSE
			- 1
			- 0 | 1
			- Enable/disable pausing/resuming threads



How to configure
----------------


