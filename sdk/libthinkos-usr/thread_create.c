#include <thinkos.h>

int thinkos_thread(int hint) 
{
	return thinkos_obj_thread_alloc(hint);
}

void __attribute__((noreturn)) thread_at_exit(int code)
{
	thinkos_exit(code);
	for(;;);
}

int __attribute__((noinline)) thinkos_thread_create_inf(thinkos_task_t task_ptr, void * task_arg,
							  const struct thinkos_thread_inf * inf)
{
	struct thinkos_thread_initializer init;
	uintptr_t stack_base = (uintptr_t)inf->stack_ptr;
	uintptr_t stack_top;
	uint32_t stack_size = inf->stack_size;
	int thread;
	int hint;
	int ret;

	hint = inf->thread_id;
	if ((thread = thinkos_obj_thread_alloc(hint)) < 0) {
		return thread;
	}

	stack_top = stack_base + stack_size;  
	/* ensure alignement */
	stack_size &= ~0xf;
	stack_top &= ~0xf;
	stack_base = stack_top - stack_size;  


	init.stack_base = stack_base;
	init.stack_size = stack_size;
	init.task_entry = (uintptr_t)task_ptr;
	init.task_exit = (uintptr_t)thread_at_exit;
	init.task_arg[0] = (uintptr_t)task_arg;
	init.task_arg[1] = thread;
	init.task_arg[2] = thread;
	init.task_arg[3] = thread;
	init.priority = inf->priority;
	init.paused = inf->paused;
	init.privileged = false;
	init.inf = inf;

	if ((ret = thinkos_thread_init(thread, &init)) < 0) {
		thinkos_obj_free(thread);
		return ret;
	}

	return thread;
}

int __attribute__((noinline)) thinkos_thread_create(thinkos_task_t task_ptr, void * task_arg, 
						  void * stack_ptr, unsigned int opt) 
{
	struct thinkos_thread_initializer init;
	uint32_t stack_size = opt & 0xffff;
	int priority = (opt >> 16) & 0xff;
	int hint = (opt >> 24) & 0x3f;
	int paused = (opt >> 31) & 1;
	uintptr_t stack_base =  (uintptr_t)stack_ptr;
	uintptr_t stack_top;
	int thread;
	int ret;

	if ((thread = thinkos_obj_thread_alloc(hint)) < 0) {
		return thread;
	}

	stack_top = stack_base + stack_size;  
	/* ensure alignement */
	stack_size &= ~0xf;
	stack_top &= ~0xf;
	stack_base = stack_top - stack_size;  

	init.stack_base = stack_base;
	init.stack_size = stack_size;
	init.task_entry = (uintptr_t)task_ptr;
	init.task_exit = (uintptr_t)thread_at_exit;
	init.task_arg[0] = (uintptr_t)task_arg;
	init.task_arg[1] = thread;
	init.priority = priority;
	init.paused = paused;
	init.privileged = false;
	init.inf = NULL;

	if ((ret = thinkos_thread_init(thread, &init)) < 0) {
		thinkos_obj_free(thread);
		return ret;
	}

	return thread;
}
