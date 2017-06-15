#ifndef TYPES_H
#define TYPES_H

#include <clist.h>
#include <uARMtypes.h>
#include <const.h>

//typedef unsigned int size_t;
typedef unsigned int pid_t;
typedef unsigned int cputime_t;

typedef struct {
	state_t *old_area;
	state_t *new_area;
} exhl_area_t;

typedef struct {
	state_t old_area;
	state_t new_area;
} pcb_area_t;


struct semd_t;
/* process control block type */
typedef struct pcb_t {
	struct pcb_t *p_parent; /* pointer to parent */
	struct semd_t *p_cursem; /* pointer to the semd_t on
				    which process blocked */
	size_t res_requested; /*resources requested from the semd_t where process is blocked  */
	pid_t p_pid;
	state_t p_s; /* processor state */
	state_t p_excpvec[EXCP_COUNT]; /*exception states vector*/
	struct clist p_list; /* process list */
	struct clist p_children; /* children list entry point*/
	struct clist p_siblings; /* children list: links to the siblings */
	cputime_t p_global_time;
	cputime_t p_user_time;
	cputime_t p_time_slice;
	pcb_area_t sysbp; /* SYS/Bp old e new area */
	pcb_area_t pgmtr; /* PgmTrap old e new area */
	pcb_area_t tlb; /* TLB old e new area */
} pcb_t;



#endif
