#include <thinkos.h>

int thinkos_mutex_alloc(void) 
{
	return thinkos_obj_alloc(THINKOS_OBJ_MUTEX);
}

int thinkos_mutex_free(int obj) 
{
	return thinkos_obj_free(obj);
}


int thinkos_cond_alloc(void) 
{
	return thinkos_obj_alloc(THINKOS_OBJ_COND);
}

int thinkos_cond_free(int cond) 
{
	return thinkos_obj_free(cond);
}

int thinkos_sem_alloc(unsigned int val) 
{
	int sem;

	if ((sem = thinkos_obj_alloc(THINKOS_OBJ_SEMAPHORE)) < 0)
		return sem;

	thinkos_sem_init(sem, val);

	return sem;
}

int thinkos_sem_free(int sem) 
{
	return thinkos_obj_free(sem);
}

int thinkos_ev_alloc(void) 
{
	return thinkos_obj_alloc(THINKOS_OBJ_EVENT);
}

int thinkos_ev_free(int set) 
{
	return thinkos_obj_free(set);
}


int thinkos_flag_alloc(void) 
{
	return thinkos_obj_alloc(THINKOS_OBJ_FLAG);
}

int thinkos_flag_free(int flag) 
{
	return thinkos_obj_free(flag);
}

int thinkos_gate_alloc(void) 
{
	return thinkos_obj_alloc(THINKOS_OBJ_GATE);
}

int thinkos_gate_free(int gate) 
{
	return thinkos_obj_free(gate);
}

