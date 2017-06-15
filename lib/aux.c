/* this file contains implementation of auxiliary functions */
#include <aux.h>
#include <types.h>
#include <libuarm.h> 
#include <init.h>
#include <debug.h>


void *a_memset(void *s, unsigned char c, int n) {
	unsigned char* p=s;
	while(n--)
		*p++ = c;
	return s;
}

int isDev (int *semaddr)
{
	return ( (semaddr == &g_pc_timer_s) || ( (semaddr >= (int *)g_dev_s) && (semaddr <= &g_dev_s[N_DEV_TYPES][N_DEV_PER_IL]) )  );
}

void onHandlerEnter(unsigned int currentTime)
{
	cputime_t elapsed = currentTime - g_lastUpdate;
	g_leftTo100 = g_leftTo100 - elapsed;

	if(g_leftTo100 > SCHED_PSEUDO_CLOCK) g_leftTo100 = 0;

	g_lastUpdate = currentTime;
	if(g_current_process!=NULL){

		g_current_process->p_global_time += elapsed;
		g_current_process->p_user_time += elapsed;
		g_current_process->p_time_slice -= elapsed;
	}
	if(g_current_process!=NULL){
		(g_leftTo100 < g_current_process->p_time_slice)? 
			setTIMER(g_leftTo100) : setTIMER(g_current_process->p_time_slice);
	}
	else
		setTIMER(g_leftTo100);
}

void onHandlerExit(unsigned int currentTime, int flag)
{
	cputime_t elapsed = currentTime - g_lastUpdate;
	
	if(flag == 1)
	{
		if(g_current_process!=NULL)
			g_current_process->p_global_time += elapsed;
	}
	
	g_leftTo100 = g_leftTo100 - elapsed;

	if(g_leftTo100 > SCHED_PSEUDO_CLOCK) g_leftTo100 = 0;

	g_lastUpdate = currentTime;
	if(g_current_process!=NULL){
		(g_leftTo100 < g_current_process->p_time_slice)? 
			setTIMER(g_leftTo100) : setTIMER(g_current_process->p_time_slice);
	}
	else
		setTIMER(g_leftTo100);
}

void copyState(state_t *src, state_t *dst) {
        dst->a1 = src->a1;
        dst->a2 = src->a2;
        dst->a3 = src->a3;
        dst->a4 = src->a4;
        dst->v1 = src->v1;
        dst->v2 = src->v2;
        dst->v3 = src->v3;
        dst->v4 = src->v4;
        dst->v5 = src->v5;
        dst->v6 = src->v6;
        dst->sl = src->sl;
        dst->fp = src->fp;
        dst->ip = src->ip;
        dst->sp = src->sp;
        dst->lr = src->lr;
        dst->pc = src->pc;
        dst->cpsr = src->cpsr;
        dst->CP15_Control = src->CP15_Control;
        dst->CP15_EntryHi = src->CP15_EntryHi;
        dst->CP15_Cause = src->CP15_Cause;
        dst->TOD_Hi = src->TOD_Hi;
        dst->TOD_Low = src->TOD_Low;
}

void memcopy(void *dest, void *src, int n)
{	
	int i;
	char *csrc = (char *)src;
	char *cdest = (char *)dest;
 
	for (i=0; i<n; i++)
	cdest[i] = csrc[i];
}
