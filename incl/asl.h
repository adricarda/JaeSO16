#ifndef _SEMAPHORE_H
#define _SEMPAHORE_H

#include "clist.h"
#include "pcb.h"
#include "const.h"

struct semd_t
{
        int *s_semAdd; /* pointer to the semaphore */
        struct clist s_link; /* ASL linked list */
        struct clist s_procq; /* blocked process queue */
};


/* insert p at the tail of of the process queue associated with the semaphore
whose physical address is semAdd. Return 1 if there are no free semaphore, 0 otherwise.
*/
int insertBlocked(int *semAdd, struct pcb_t *p);

/* search the semaphore semAdd in the ASL list. */
/* return NULL if none is found, the first pcb of the found sempahore otherwise.*/
struct pcb_t *removeBlocked(int *semAdd);

/*remove the pcb pointed to by p from the process queue associated
with p’s semaphore on the ASL */
struct pcb_t *outBlocked(struct pcb_t *p);

/*return NULL if semAdd is not found. 
  return the pcb that is at the head of the process queue associated with the semaphore semAdd. */
struct pcb_t *headBlocked(int *semAdd);

/* initialize the semdFree list to contain all the elements of the array semdTable*/
void initASL(void);

#endif
