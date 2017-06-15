#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#include <uARMtypes.h>
#include <libuarm.h>
#include <const.h>
#include <types.h>
#include <aux.h>
#include <init.h>
 
/* causes a new process, said to be a progeny of the caller, to be created */
void createProcess(state_t *npstate);

/* causes the executing process or one process in its progeny to cease to exist */
void terminateProcess(pid_t pid);

/* performs a weighted operation on a semaphore */
void semop( int *semaddr, int weight);

/* prepares the System Call/Breakpoint New Area of the calling process for exception pass-up */
void specSysHdl( memaddr pc, memaddr sp, unsigned int flags);

/* prepares the TLB Exception New Area of the calling process for exception pass-up */
void specTLBHdl( memaddr pc, memaddr sp, unsigned int flags);

/* prepares the Program Trap New Area of the calling process for exception pass-up */
void specPgmTHdl( memaddr pc, memaddr sp, unsigned int flags);

/* causes the processor status stored in one of the three ProcBlk Old Areas to be loaded in order to return from 
a higher level handler function */
void exitTrap( unsigned int excptype, unsigned int retval);

/* causes the processor time (in microseconds) used by the requesting process, both as global time and user time,
to be returned to the calling process */
void getCpuTime( cputime_t *global, cputime_t *user);

/* locks the requesting process on the nucleus maintained pseudo-clock timer semaphore */
void waitClock();

/* performs the operation indicated by the value in a2 and then locks the requesting process on the semaphore 
that the nucleus maintains for the I/O device indicated by the values in a3, a4 */
void ioDevop( unsigned int command, int int1No, unsigned int dnum);

/* returns the Process ID of the caller by placing its value in a1 */
void getPid();

/* SYSTEM CALL AND BREAKPOINT EXCEPTION HANDLER */
void sysBP_Handler();

/* PROGRAM TRAP EXCEPTION HANDLER */
void pgmTrap_Handler();

/* TLB EXCEPTION HANDLER */
void TLB_Handler();

#endif
