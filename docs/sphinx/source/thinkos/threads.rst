
Threads
=======

.. describe:: #include <thinkos.h>

.. default-domain:: c

Error Codes
-----------

Most of system and libraries will return either c:member:`THINKOS_OK` or a positive value upon success. In case of failuere the call can either retun one of errors described in c:type:`enum thinkos_err` or raise an exception wich will be hadled by the debug monitor if configured with this option. 

.. c:type:: enum thinkos_err

	.. list-table::
		:widths: 50 10 100
		:header-rows: 1
		:align: left 

		*	- Member
			- Value
			- Description

		*	- .. c:member:: THINKOS_OK        
			- 0
			- No error

		*	- .. c:member:: HINKOS_ETIMEDOUT
			- -1
			- System call timed out

		*	- .. c:member:: THINKOS_EINTR
			- -2
			- System call interrupted

		*	- .. c:member:: THINKOS_EINVAL
			- -3
			- Invalid argument

		*	- .. c:member:: THINKOS_EAGAIN
			- -4
			- Non blocking call failed

		*	- .. c:member:: THINKOS_EDEADLK
			- -5
			- Deadlock condition detected

		*	- .. c:member:: THINKOS_EPERM
			- -6
			- Invalid permission 

		*	- .. c:member:: THINKOS_ENOSYS
			- -7
			- Invalid system call

		*	- .. c:member:: THINKOS_EFAULT
			- -8
			- Internal System Fault 

		*	- .. c:member:: THINKOS_ENOMEM
			- -9
			- Resource pool exausted

.. c:type:: struct cortex_m_context

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

	The C domain of the kernel-doc has some additional features. E.g. you can
	*rename* the reference name of a function with a common name like ``open`` 
	or ``ioctl``:


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

.. c:function::  register_ftrace_function(&ops);

To disable tracing call:

.. c:function::  unregister_ftrace_function(&ops);

The above is defined by including the header:

.. c:function:: #include <linux/ftrace.h>

The registered callback will start being called some time after the
register_ftrace_function() is called and before it returns. The exact time
that callbacks start being called is dependent upon architecture and scheduling
of services. The callback itself will have to handle any synchronization if it
must begin at an exact moment.

The unregister_ftrace_function() will guarantee that the callback is
no longer being called by functions after the unregister_ftrace_function()
returns. Note that to perform this guarantee, the unregister_ftrace_function()
may take some time to finish.


The callback function
---------------------

The prototype of the callback function is as follows (as of v4.14):

.. c:function:: void callback_func(unsigned long ip, unsigned long parent_ip, 
	struct ftrace_ops * op, struct pt_regs * regs);
	:noindex:

.. param ip: This is the instruction pointer of the function that is being traced. (where the fentry or mcount is within the function)

@parent_ip
	This is the instruction pointer of the function that called the
	the function being traced (where the call of the function occurred).

@op
	This is a pointer to ftrace_ops that was used to register the callback.
	This can be used to pass data to the callback via the private pointer.

@regs
	If the FTRACE_OPS_FL_SAVE_REGS or FTRACE_OPS_FL_SAVE_REGS_IF_SUPPORTED
	flags are set in the ftrace_ops structure, then this will be pointing
	to the pt_regs structure like it would be if an breakpoint was placed
	at the start of the function where ftrace was tracing. Otherwise it
	either contains garbage, or NULL.


