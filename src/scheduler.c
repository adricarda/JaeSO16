#include <pcb.h>
#include <aux.h>
#include <libuarm.h>
#include <init.h>

void scheduler () 
{
	unsigned int tod;
	unsigned int current_status;
	if (headProcQ(&g_ready_queue))
	{
		/*there is at least one elemet*/
		g_current_process = removeProcQ(&g_ready_queue);
		g_current_process->p_time_slice = (cputime_t)SCHED_TIME_SLICE;
		tod = (unsigned int)getTODLO();
		onHandlerExit(tod,0);
		debugBKtwo((unsigned int)g_current_process->p_pid );
		LDST(&g_current_process->p_s); /*leaves the control to the process*/
	}
	else
	{
		if (g_process_count<=0 || g_process_count > MAXPROC  )
			HALT(); /*the ready_queue is empty and no process are waiting for I/O*/
		else 
			if (g_soft_block_count>0)
			{
				current_status = getSTATUS() | STATUS_SYS_MODE; /*enable kernel-mode*/
				current_status = STATUS_ALL_INT_ENABLE(current_status); /*enable local timer and interrupts */
				setSTATUS(current_status);
				tod = (unsigned int)getTODLO();
				onHandlerExit(tod,0);
				WAIT();
			}
			else 
				PANIC(); /*DEADLOCK */
		}
}
