#ifndef _PCB_H
#define _PCB_H

#include <clist.h>
#include <aux.h>
#include <types.h>
#include <uARMconst.h>
//#define NULL 0
struct semd_t;

/* initialize the pcbFree list  */
void initPcbs(void);

/* returns NULL if the pcbFree list is empty, otherwhise returns a pointer to the new element from pcbFree  */
struct pcb_t *allocPcb(void);

/* returns prcBlk to the freePcb list  */
void freePcb(struct pcb_t *p);

/* it inserts the ProcBlk pointed to by p into the process queue whose list-tail pointer is q */
void insertProcQ (struct clist *q, struct pcb_t *p);

/* it removes the first element from the process queue whose list-tail pointer is q */ 
/* it returns NULL if the process queue was initially empty; otherwise it returns the pointer to the removed element */
struct pcb_t* removeProcQ (struct clist *q);

/* it removes the ProcBlk pointed to by p from the process queue whose list-tail
pointer is q */
/* if the desired entry is not in the indicated queue, it returns NULL; otherwise, return p */
struct pcb_t *outProcQ (struct clist *q, struct pcb_t *p);

/* it returns a pointer to the first ProcBlk from the process queue whose list-
tail pointer is q with no remotion of this ProcBlk from the process queue */
/* it returns NULL if the process queue is empty */
struct pcb_t *headProcQ (struct clist *q);

/* checks if the child list of p is empty */
/* returns TRUE if it is, FALSE otherwise */
int emptyChild(struct pcb_t *p);

/* add p, as the last element, to child list of parent */
void insertChild(struct pcb_t *parent, struct pcb_t *p);

/* remove the first child of p and return a pointer to it */
/* if child list of p is empty nothing is return */
struct pcb_t *removeChild(struct pcb_t *p);

/* makes p no longer a child of its parent and return a
   pointer to it */
/* if p has no parent, nothing is return */
struct pcb_t *outChild(struct pcb_t *p);

/* return child of p whose pid match the provided one*/
/* if p has no child or no pid match NULL is return*/
struct pcb_t *childByPid(struct pcb_t *p, unsigned int pid);
#endif
