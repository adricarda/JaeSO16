/*
This module implements the Syscall/Breakpoint handler, the Program Trap handler
and the TLB exception handler. This module will handle the exceptions raised
up by the System Calls, the BreakPoints, the Program Traps (illegal and
undefined actions like the reserved istructions) and the TLB (failed attempts
to translate a virtual address into its corresponding physical address).
*/

#include <exceptions.h>
#include <debug.h>

#define SBK_HDL 16
#define TLB_HDL 17
#define PGM_HDL 18

unsigned int abs_process_count = 1;
unsigned int bySysHdl = 0;

state_t *m_sysbold = (state_t*)SYSBK_OLDAREA;
state_t *m_pgmold = (state_t*)PGMTRAP_OLDAREA;
state_t *m_tlbold = (state_t*)TLB_OLDAREA;

/* state will be loaded as current processor state at the exit of the Handlers */
state_t *m_state_loaded;

HIDDEN pid_t pid_selector() {
	pid_t id;
	abs_process_count++;
	id = abs_process_count << 5;
	id |= g_process_count; 
	return id;	
}

HIDDEN void terminateProgeny(struct pcb_t *progeny) {
	outChild(progeny); /*orphaned from its parent*/
	while (!emptyChild(progeny)) terminateProgeny(removeChild(progeny)); /*terminate all its child before*/
	if (progeny->p_cursem) { /*process is waiting on a semaphore or it's in ready_queue*/
		if (isDev((int *)progeny->p_cursem)) /*the semaphore is a device one*/
			g_soft_block_count--;
		outBlocked(progeny);
	} 
	else outProcQ(&g_ready_queue, progeny);
	freePcb(progeny);
	g_process_count--;
}

HIDDEN void commonSys( int check, memaddr pc, memaddr sp, unsigned int flags) {
	
	state_t *pToNewArea;	

	switch (check) {
		case SBK_HDL:	/* SYS4 */
			pToNewArea = &( g_current_process->sysbp.new_area);
			break;
		case TLB_HDL:	/* SYS5 */
			pToNewArea = &( g_current_process->tlb.new_area);
			break;
		case PGM_HDL:	/* SYS6 */
			pToNewArea = &( g_current_process->pgmtr.new_area);
			break;
		default:
			break;
	}

	if ( pToNewArea->pc == 0) {	/* if true, SYS not issued yet */
		/* a2 (handler address), a3 (stack address) copied from PgmTrap Old Area to ProcBlk PgmTrap New Area's pc, sp */
		pToNewArea->pc = pc;
		pToNewArea->sp = sp;
		
		/* New Area virtual memory mode set according to the flags[31] */
		if (flags >> 31)	/* if true, flag[31] = 1 (virtual memory enabled) */
			pToNewArea->CP15_Control = CP15_ENABLE_VM( pToNewArea->CP15_Control);
		else pToNewArea->CP15_Control = CP15_DISABLE_VM( pToNewArea->CP15_Control);		
		
		/* flags[0-7] copied into first byte of New Area cpsr */
		flags &= 0xff;
		pToNewArea->cpsr |= flags;

		/* New Area ASID is the same as the calling process */
		pToNewArea->CP15_EntryHi = ENTRYHI_ASID_SET( pToNewArea->CP15_EntryHi, ENTRYHI_ASID_GET( g_current_process->p_s.CP15_EntryHi));
	}
	else terminateProcess(0);
	
}

/* SYS1 */
void createProcess(state_t *npstate) {
	if (g_process_count < MAXPROC) {
		struct pcb_t *new_pcb = allocPcb();
		g_process_count++;
		new_pcb->p_pid = pid_selector();
		copyState( npstate, &(new_pcb->p_s));
		insertChild(g_current_process, new_pcb);
		insertProcQ(&g_ready_queue, new_pcb);
		m_sysbold->a1 = new_pcb->p_pid; /* return pid */
	}
	else m_sysbold->a1 = -1; /* return error */

}

/* SYS2 */
void terminateProcess(pid_t pid) {
	if (pid != 0) { /* wants delete child */
		if (!emptyChild(g_current_process)) { /* current process have childs */
			struct pcb_t *child = childByPid(g_current_process, pid); /* looking for current process child with right pid */
			if (child) { /* if exist terminate it */
				terminateProgeny(child);
			}
		}
	}
	else { /*want delete current process*/
		terminateProgeny(g_current_process);
		g_current_process = NULL;
	}
}


/* SYS3 */
void semop( int *semaddr, int weight) {
	struct pcb_t *head;
	int freeResources = 0;
	int flag = 1;
	if (weight > 0) {
		freeResources = weight + *semaddr;
		/* freeResources represents the number of resources can be assigned to the 
		next stucked process on the semaphore in the current instant*/ 
		while ( (( head=headBlocked(semaddr)) != NULL) && (flag)) {
			if (freeResources >= head->res_requested) {
				freeResources -= head->res_requested;				
				head->res_requested = 0;
				removeBlocked(semaddr);
				insertProcQ(&g_ready_queue,head);
				if (isDev( (int *)(head->p_cursem->s_semAdd)))
					g_soft_block_count--;
				head=headBlocked(semaddr);				
			}
			else flag = 0;
		}
		/* if there isn't a stucked process (freeResources value will be different from 0) 
		memorizes verhogen operation on the semaphore, else assigns as many resources as possible 
		and awakens process with the request satisfied */
		*semaddr = freeResources;
	}
	else if (weight <0) {
		if ( (headBlocked(semaddr) != NULL) || ((*semaddr + weight) < 0)) { 
		/* there is yet another stuck process on the semaphore or semaphore 
		resources are sufficient to meet the process's demand */
				if (insertBlocked(semaddr,g_current_process) == 0) {
					g_current_process->res_requested= -weight;
					onHandlerExit( getTODLO(), 1);
					g_current_process = NULL;		
					if (isDev( (int *)(semaddr)))
						g_soft_block_count++;	
				}	
				else 	/* there are no free semaphores */
					PANIC();
		}
		else *semaddr -= -weight;
	}	
	else PANIC();
}

/* SYS4 */
void specSysHdl( memaddr pc, memaddr sp, unsigned int flags) {
	
	commonSys( SBK_HDL, pc, sp, flags);

}

/* SYS5 */
void specTLBHdl( memaddr pc, memaddr sp, unsigned int flags) {

	commonSys( TLB_HDL, pc, sp, flags);	

}

/* SYS6 */
void specPgmTHdl( memaddr pc, memaddr sp, unsigned int flags) {

	commonSys( PGM_HDL, pc, sp, flags);	

}

/* SYS7 */
void exitTrap( unsigned int excptype, unsigned int retval) {
	state_t *tmp_pointer;
	switch ( excptype) {
		case 0 : 	/* Sys/BP exception type */
			/* optional return value in retval copied in ProcBlk */
			/* SYS/BP Old Area that will be loaded to processor */
			tmp_pointer = &( g_current_process->sysbp.old_area);		
			break;
		case 1 : 	/* TLB exception type */
			tmp_pointer = &( g_current_process->tlb.old_area);
			break;
		case 2 : 	/* PgmTrap exception type */
			tmp_pointer = &( g_current_process->pgmtr.old_area);
			break;
		default : 	/* error occurred */
			PANIC();
			break;	
	}
	tmp_pointer->a1 = retval;
	/* state will will be loaded as processor state at the exit of the Sys/BP Handler*/	
	m_state_loaded = tmp_pointer;
}

/* SYS8 */
void getCpuTime( cputime_t *global, cputime_t *user) {
	/* return in microseconds */

	*global = g_current_process->p_global_time;
	*user = g_current_process->p_user_time;
}

/* SYS9 */
void waitClock() {
	semop( &g_pc_timer_s, -1);
	g_pseudoClockBlk++;

}

/* SYS10 */
void ioDevop( unsigned int command, int int1No, unsigned int dnum) {
	devreg_t *dev_pointer;
	unsigned int dnumT = dnum;
	unsigned int semIndex;
	
	if (int1No == INT_TERMINAL) {	/* int1No (line), if true, terminal device */
		dnumT <<= 1;
		dnumT >>= 1;
		dev_pointer = (devreg_t*)(DEV_REG_ADDR(int1No, dnumT));

		if (dnum >> 7) /* if true, READ operation, else WRITE */
			semIndex = 5;
		else semIndex = 4;

		semop( &g_dev_s[semIndex][dnumT], -1);

		if (dnum >> 7) /* if true, READ operation, else WRITE */
			dev_pointer->term.recv_command = command;
		else dev_pointer->term.transm_command = command;

	}
	else {	/* normal device */
		semIndex = int1No - DEV_IL_START;
		dev_pointer = (devreg_t*)(DEV_REG_ADDR(int1No, dnumT));
		semop( &g_dev_s[semIndex][dnumT], -1);
		dev_pointer->dtp.command = command;
	}

}

/* SYS11 */
void getPid() {
	/* current process pid copied in a1 register */
	m_sysbold->a1 = g_current_process->p_pid;
}

/* DEFAULT SYSTEM CALLS (1-11) FUNCTION */
HIDDEN void defSys() {
	switch ( m_sysbold->a1)	{	
		case CREATEPROCESS: 
			createProcess( (state_t *)(m_sysbold->a2));
			break;
		case TERMINATEPROCESS: 
			terminateProcess( (pid_t)(m_sysbold->a2));
			break;
		case SEMOP: 
			semop( (int *)(m_sysbold->a2), (int)(m_sysbold->a3));
			break;
		case SPECSYSHDL: 
			specSysHdl( m_sysbold->a2, m_sysbold->a3, m_sysbold->a4);
			break;
		case SPECTLBHDL: 
			specTLBHdl( m_sysbold->a2, m_sysbold->a3, m_sysbold->a4);
			break;
		case SPECPGMTHDL: 
			specPgmTHdl( m_sysbold->a2, m_sysbold->a3, m_sysbold->a4);
			break;
		case EXITTRAP: 
			exitTrap( m_sysbold->a2, m_sysbold->a3);
			break;
		case GETCPUTIME: 
			getCpuTime( (cputime_t *)(m_sysbold->a2), (cputime_t *)(m_sysbold->a3));
			break;
		case WAITCLOCK: 
			waitClock();
			break;
		case IODEVOP: 
			ioDevop( m_sysbold->a2, (int)(m_sysbold->a3), m_sysbold->a4);
			break;
		case GETPID: 
			getPid();
			break;						
	}
}

/* SUB-HANDLER FOR PGMTRAP AND TLB EXCEPTIONS */
HIDDEN void commonSubHandler( int check) {
	
	pcb_area_t *pcbArea;
	state_t *romArea;

	if (bySysHdl)
		bySysHdl = 0;
	else onHandlerEnter( getTODLO());
	
	if ( check == TLB_HDL) {	/* TLB HANDLER */
		pcbArea = &g_current_process->tlb;	
		romArea = m_tlbold;
	}
	else if ( check == PGM_HDL) {	/* PGMTRAP HANDLER */		
		pcbArea = &g_current_process->pgmtr;	
		romArea = m_pgmold;
	}

	/* processor state saved in Pcb state_t */	
	copyState( romArea, &(g_current_process->p_s));
	
	if ( pcbArea->new_area.pc == 0) {	/* if true, SYS not issued */
		/* current process and all its progeny are terminated */
		/* then will be called the scheduler */		
		terminateProcess(0);
	} 
	else {
		/* processor state stored in ROM Old Area is moved to ProcBlk Old Area */
		copyState( romArea, &(pcbArea->old_area));
		/* Cause register copied from ROM Old Area to ProcBlk New Area a1 register */
		pcbArea->new_area.a1 = romArea->CP15_Cause;
		/* state will be loaded as processor state at the exit of the Handler*/
		m_state_loaded = &(pcbArea->new_area);
	}

	onHandlerExit( getTODLO(), 0);
}

/* SYSTEM CALLS ABOVE 11 AND BREAKPOINT FUNCTION */
HIDDEN void sysAboveBP() {
	state_t *tmp_pointer = &( g_current_process->sysbp.new_area);
	unsigned int tmp;
	if ( tmp_pointer->pc == 0) {	/* if true, SYS4 not issued */
		/* current process and all its progeny are terminated */
		/* then will be called the scheduler */			
		terminateProcess(0);
	} 
	else {
		/* processor state stored in SYS/BP Old Area is moved to ProcBlk SYS/BP Old Area */
		copyState( m_sysbold, &(g_current_process->sysbp.old_area));		
		/* a1-a4 register copied from SYS/BP Old Area to ProcBlk SYS/BP New Area */
		tmp_pointer->a1 = m_sysbold->a1;
		tmp_pointer->a2 = m_sysbold->a2;
		tmp_pointer->a3 = m_sysbold->a3;
		tmp_pointer->a4 = m_sysbold->a4;

		/* lower 4 bits of SYS/BP Old Area's cpsr reg. is copied on most significant positions of ProcBlk SYS/BP New Area's a1 reg. */		
		tmp = m_sysbold->cpsr << 28;
		tmp_pointer->a1 &= 0xFFFFFFF;

		tmp_pointer->a1 |= tmp;		

		//copybit( &tmp, &(tmp_pointer->a1), 28, 32);		
	
		/* state will will be loaded as processor state at the exit of the Sys/BP Handler*/		
		m_state_loaded = tmp_pointer;
	}	
}

/* SYSTEM CALL AND BREAKPOINT EXCEPTION HANDLER */
void sysBP_Handler() {

	/* default state (executing process) will be loaded as processor state at the exit of the Sys/BP Handler */
	m_state_loaded = m_sysbold;
	unsigned int exception_raised = CAUSE_EXCCODE_GET(m_sysbold->CP15_Cause);
	
	/* processor state saved in Pcb state_t */
	copyState( m_sysbold, &(g_current_process->p_s));
	
	if ( exception_raised == EXC_SYSCALL) {	/* if true, it's a SYSCALL */
		
		onHandlerEnter( getTODLO());	
		if (( m_sysbold->a1 >= CREATEPROCESS) && ( m_sysbold->a1 <= GETPID)) {
			if ( KERNEL_MODE( m_sysbold->cpsr)) {	/* if true, kernel mode execution, the first 5 bits of CPSR = 0x1F */
				defSys();
			}
			else {
				/* program trap exception raised up */
				copyState( m_sysbold, m_pgmold);				
				m_pgmold->CP15_Cause = CAUSE_EXCCODE_SET( m_pgmold->CP15_Cause, EXC_RESERVEDINSTR);
				
				bySysHdl = 1;

				onHandlerExit( getTODLO(), 1);
				pgmTrap_Handler();
			}
		}
		else sysAboveBP();

		onHandlerExit( getTODLO(), 1);
		
	}
	else if ( exception_raised == EXC_BREAKPOINT) {	/* if true, it's a Breakpoint */
		
		onHandlerEnter( getTODLO());	
		
		sysAboveBP();		
		
		onHandlerExit( getTODLO(), 0);
	}
	/* if not SYSCALL or Breakpoint then error */
	else PANIC();

	if ( g_current_process == NULL)	/* if true, SYS2 issued */
		scheduler();
	else LDST( m_state_loaded);
}

/* PROGRAM TRAP EXCEPTION HANDLER */
void pgmTrap_Handler() {

	commonSubHandler( PGM_HDL);
	
	if ( g_current_process == NULL)	/* if true, SYS2 issued */
		scheduler();
	else LDST( m_state_loaded);
}

/* TLB EXCEPTION HANDLER */
void TLB_Handler() {

	commonSubHandler( TLB_HDL);
	
	if ( g_current_process == NULL)	/* if true, SYS2 issued */
		scheduler();
	else LDST( m_state_loaded);
}


