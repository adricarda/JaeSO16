#include <init.h>
#include <uARMconst.h>
#include <arch.h>

extern void test();
extern void scheduler();

HIDDEN void init_new_area(exhl_area_t area, memaddr handler) {
	area.new_area->pc = handler;
	area.new_area->sp = RAM_TOP;
	area.new_area->cpsr = STATUS_ALL_INT_DISABLE(STATUS_SYS_MODE);
}

int main() {
	char i, j;
	struct pcb_t *init_process;	

	int_a.old_area = (state_t*)INT_OLDAREA;
	int_a.new_area = (state_t*)INT_NEWAREA;
	tlb_a.old_area = (state_t*)TLB_OLDAREA;
	tlb_a.new_area = (state_t*)TLB_NEWAREA;
	trp_a.old_area = (state_t*)PGMTRAP_OLDAREA;
	trp_a.new_area = (state_t*)PGMTRAP_NEWAREA;
	sysbp_a.old_area = (state_t*)SYSBK_OLDAREA;
	sysbp_a.new_area = (state_t*)SYSBK_NEWAREA;

	/*Populating ROM Reserved Frame's new areas*/
	init_new_area(int_a, (memaddr)InterruptExceptionHandler);
	init_new_area(tlb_a, (memaddr)TLB_Handler);
	init_new_area(trp_a, (memaddr)pgmTrap_Handler);
	init_new_area(sysbp_a, (memaddr)sysBP_Handler);
	
	/*Initializing pcb and semaphore free list*/
	initPcbs();
	initASL();

	/*Initializing nucleus maintained variables and devices semaphores variables*/
	g_process_count = 1;
	g_soft_block_count = 0;
	g_ready_queue.next = NULL; //CLIST_INIT
	g_current_process = NULL;
	g_lastUpdate = 0;
	g_pseudoClockBlk = 0;
	g_leftTo100 = SCHED_PSEUDO_CLOCK;
	for (i = 1; i<N_DEV_TYPES; i++) {
		for (j = 1; j<N_DEV_PER_IL; j++) {
			g_dev_s[i][j] = 0;
		}
	}
	g_pc_timer_s = 0;

	/*First process creation*/
	init_process = allocPcb();
	init_process->p_pid = 1;
	init_process->p_parent = NULL;
	init_process->p_cursem = NULL;
	/*VM off by default*/
	init_process->p_s.cpsr = STATUS_ALL_INT_ENABLE(STATUS_SYS_MODE); /*in Kernel Mode IRQ enabled Local Timer enabled*/
	init_process->p_s.sp = RAM_TOP - FRAMESIZE; /*set to secondlast RAM Frame*/
	init_process->p_s.pc = (memaddr) test; /*pc to function test phase2*/
	insertProcQ(&g_ready_queue, init_process);
	init_process->p_children.next = NULL; //CLIST_INIT
	init_process->p_siblings.next = NULL; //CLIST_INIT

	scheduler();
}
