#ifndef _INTERRUPT_C_
#define _INTERRUPT_C_

#include <arch.h>
#include <libuarm.h> 
#include <uARMconst.h>
#include <uARMtypes.h>
#include <interrupt.h>
#include <pcb.h>
#include <types.h>
#include <const.h>
#include <aux.h>
#include <init.h>

/*
This module implements the device interrupt exception
handler. This module will process all the device interrupts, including
Interval Timer interrupts, converting device interrupts into operations on
the appropriate semaphores.
*/


/*Won't happen, added for completeness.*/ 
static void handleIPI()
{
}
/*Won't happen, added for completeness. */
static void handleCpuTimer()
{
}

void handleTimer()
{
	cputime_t currentTime,elapsed;	
	currentTime = getTODLO();
	elapsed = currentTime - g_lastUpdate;
	/*Check if pseudo clock timer has expired. */
	if(g_leftTo100  == elapsed || g_leftTo100-elapsed > SCHED_PSEUDO_CLOCK)
	{
		
		g_leftTo100 = SCHED_PSEUDO_CLOCK;
		while(g_pseudoClockBlk > 0)
		{
			semop(&g_pc_timer_s, g_pseudoClockBlk);
			g_pseudoClockBlk--;
		}
			
	}
	/*Check if time slice has expired. */
	if( g_current_process->p_time_slice > SCHED_TIME_SLICE )
	{
		/*Handle the end of the time slice.*/
		onHandlerExit( getTODLO(), 1);
    		insertProcQ(&g_ready_queue, g_current_process);
		g_current_process = NULL;
	}
}

void handleDevices(int line)
{
	int* bitMap;
	int dev,i;
 	dtpreg_t* reg;
	termreg_t* termReg;
	unsigned int devStatus;
	struct pcb_t* headPCBbefore;
	
	bitMap = (int*) CDEV_BITMAP_ADDR(line);

	/*Finding device number. */
	for(i = 0; i< N_DEV_PER_IL; i ++)
	{	
		if((*bitMap & 0x1) == 1){

			dev = i;
			break;
		}
		*bitMap = *bitMap >> 1 ;
	}

	/*Acknowledge the outstanding interrupt */
	if(line == IL_TERMINAL)
	{	
		termReg = (termreg_t*) (DEV_REG_ADDR(line,dev));
		if( ((termReg->recv_status & 0xFF) == DEV_TRCV_S_CHARRECV) ||
			((termReg->recv_status & 0xFF) == DEV_TRCV_S_RECVERR) )
		{
			devStatus = termReg->recv_status;
			termReg->recv_command = DEV_C_ACK;
			line = N_DEV_TYPES;
			
		}		
		else if( ((termReg->transm_status & 0xFF) == DEV_TTRS_S_CHARTRSM) ||
			 ((termReg->transm_status & 0xFF) == DEV_TTRS_S_TRSMERR) )
		{
			
			devStatus = termReg->transm_status;
			termReg->transm_command = DEV_C_ACK;
			line = line-DEV_IL_START;
			
		}	
	}
	else /*Normal devices */
	{
		reg = (dtpreg_t *) (DEV_REG_ADDR(line,dev));
		reg->command = DEV_C_ACK;
		devStatus = reg->status;
		line = line-DEV_IL_START;
	}
	

	/*SYS3 with weight 1 on the device's sem.*/
	headPCBbefore = headBlocked(&(g_dev_s[line][dev]));

	if(g_dev_s[line][dev] < 1){
		semop(&g_dev_s[line][dev], 1);
	}	

	/*se era richiesta una sys10 (perchè con la sys3 ho sbloccato un processo bloccato,
  salvo la status word del device nel registro a1 del processo sbloccato:*/
 
	if (headPCBbefore != headBlocked(&(g_dev_s[line][dev]))){
		headPCBbefore->p_s.a1 = devStatus;
	}
}

void InterruptExceptionHandler()
{
	
	unsigned int line,status;		
	int cause;
	
	state_t	*old_area;
	old_area = (state_t *)INT_OLDAREA;
	old_area->pc -= 4;

	onHandlerEnter(getTODLO());
	if(g_current_process!=NULL){
		copyState(old_area, &(g_current_process->p_s));
	}		
	cause = getCAUSE();

	/*Finding which line has an interrupt*/
	if((CAUSE_IP_GET(cause,IL_IPI)))
		handleIPI();

	else if((CAUSE_IP_GET(cause,IL_CPUTIMER)))
		handleCpuTimer();

	else if((CAUSE_IP_GET(cause,IL_TIMER)))
		handleTimer();

	else if((CAUSE_IP_GET(cause,IL_DISK)))
		handleDevices(IL_DISK);

	else if((CAUSE_IP_GET(cause,IL_TAPE)))
		handleDevices(IL_TAPE);

	else if((CAUSE_IP_GET(cause,IL_ETHERNET)))
		handleDevices(IL_ETHERNET);

	else if((CAUSE_IP_GET(cause,IL_PRINTER)))
		handleDevices(IL_PRINTER);

	else if((CAUSE_IP_GET(cause,IL_TERMINAL)))
		handleDevices(IL_TERMINAL);
		

	/*Update pseudo clock timer. */	
	onHandlerExit(getTODLO(), 0);

	/*return to the interrupted process or call the scheduler. */
	if(g_current_process != NULL)
		LDST(old_area);
	else{
		scheduler();
	}
}

#endif
