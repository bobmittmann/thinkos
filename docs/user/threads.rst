
Threads
=======

.. describe:: #include <thinkos.h>

.. default-domain:: c

Error Codes
-----------

Most of system and libraries will return either c:member:`THINKOS_OK` or a positive value upon success. In case of failuere the call can either retun one of errors described in c:type:`enum thinkos_err` or raise an exception wich will be hadled by the debug monitor if configured with this option. 


	Cortex-M context

	.. code-block:: c

		struct cortex_m_context {
			uint32_t r0;
			uint32_t r1;
			uint32_t r2;
			uint32_t r3;

			uint32_t r4;
			uint32_t r5;
			uint32_t r6;
			uint32_t r7;

			uint32_t r8;
			uint32_t r9;
			uint32_t r10;
			uint32_t r11;

			uint32_t r12;
			uint32_t sp;
			uint32_t lr;
			uint32_t pc;

			uint32_t xpsr;
		};

.. c:macro:: HELLO

	This macro says hello

.. c:macro:: WORLD

	This macro says world

.. c:function:: int ioctl( int fd, int request ) 

	The func-name (e.g. ioctl) remains in the output but the ref-name changed 
	from ``ioctl`` to ``VIDIOC_LOG_STATUS``. The index entry for this function 
	is also changed to ``VIDIOC_LOG_STATUS`` and the function can now 
	referenced by.

	:param priority: The priority of the message, can be a set to :c:macro:`HELLO` or :c:macro:`WORLD`

	:param test: :type:`uint32_t` The priority of the message, can be a number 1-5


	:returns: :type:`reference` to the first item in the string_view


The ftrace_ops structure
------------------------

To register a function callback, a ftrace_ops is required. This structure
is used to tell ftrace what function should be called as the callback
as well as what protections the callback will perform and not require
ftrace to handle.

There is only one field that is needed to be set when registering
an ftrace_ops with ftrace:

.. code-block:: c

	struct ftrace_ops ops = {
		.func			= my_callback_func,
		.flags			= MY_FTRACE_FLAGS
		.private			= any_private_data_structure,
		};

Both .flags and .private are optional. Only .func is required.

To enable tracing call:

