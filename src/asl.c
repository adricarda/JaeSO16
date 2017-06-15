#ifndef _SEMAPHORE_C
#define _SEMPAHORE_C

#include <asl.h>

/*Entry point for the list (ascending order)*/
static struct clist aslh = CLIST_INIT; 
/* used to mantain the unused sempahore descriptors */
static struct clist semdFree = CLIST_INIT;


int insertBlocked(int *semAdd, struct pcb_t *p)
{
   void* l_tmp = NULL;
   struct semd_t* l_scan;
   int flag = 0;

   /*search for the given semaphore.*/	
   clist_foreach(l_scan, &aslh, s_link, l_tmp) 
   {
	if (l_scan->s_semAdd==semAdd){
		flag =1;
		break;
	}
	else if (l_scan->s_semAdd>semAdd){
		break;
	}
   } 	
   /* if none is found allocate a new descriptor from the semdFree list. */	
   if ((flag==0)||(clist_foreach_all(l_scan, &aslh, s_link,l_tmp)))
   {
	/*If there are no free semaphore return 1.*/
	if(clist_empty(semdFree))
		return 1;	

	/*Otherwise get a new semaphore descriptor.*/
	struct semd_t* head = clist_head(l_scan, semdFree, s_link);	
	if(head == NULL)
		return 0;	

	clist_dequeue(&semdFree);

	/*insert the new descriptor at the appropriate position.*/
	clist_foreach(l_scan, &aslh, s_link, l_tmp) 
   	{
	  if (semAdd < l_scan->s_semAdd)
	  {
		clist_foreach_add(head, l_scan, &aslh, s_link, l_tmp)
		break;
	  }
   	}
	if (clist_foreach_all(l_scan, &aslh, s_link,l_tmp)) {
	  clist_enqueue(head, &aslh, s_link);
	}
	
	head->s_semAdd = semAdd;
	head->s_procq.next = NULL;
		
	p->p_cursem = head;
	/*add the pcb to the semaphore queue. */
	clist_enqueue(p, &(head->s_procq),p_list)
	
	
   }
   else
   {
	/*add the pcb to the semaphore queue. */
	clist_enqueue(p, &(l_scan->s_procq), p_list);
	p->p_cursem = l_scan;
   }

return 0;
}

struct pcb_t *removeBlocked(int *semAdd)
{
   void* l_tmp = NULL;
   struct semd_t* l_scan;
   struct pcb_t* l_result;
   int flag = 0;
   
   /*search for the given semaphore.*/	
   clist_foreach(l_scan, &aslh, s_link, l_tmp) 
   {
	if (semAdd==l_scan->s_semAdd){
		flag = 1;
		break;
	}	
	else if (l_scan->s_semAdd>semAdd){
		break;
	}
   }
   /* if none is found return null. */	
   if ((flag == 0)||(clist_foreach_all(l_scan, &aslh, s_link,l_tmp)))
	return NULL;

   /* otherwise get the first PCB of this semaphore. */
   l_result = clist_head(l_result, l_scan->s_procq, p_list);
   clist_dequeue(&(l_scan->s_procq));
   

   /* if the list becomes empty remove the descriptor from ASL*/
   if(clist_empty(l_scan->s_procq))
   {	
   	clist_delete(l_scan, &aslh, s_link);
	
	/* and insert it in the free descriptor. */
	clist_enqueue(l_scan, &semdFree, s_link);
   }
   return l_result;
}

struct pcb_t *outBlocked(struct pcb_t *p)
{
  int l_flag = 1;
  if(outProcQ(&(p->p_cursem->s_procq),p) == 0)
	l_flag = 0;

  /* if the list becomes empty remove the descriptor from ASL*/
   if(clist_empty(p->p_cursem->s_procq))
   {	
   	clist_delete(p->p_cursem, &aslh, s_link);
	
	/* and insert it in the free descriptor. */
	clist_enqueue(p->p_cursem, &semdFree, s_link);
   }
	
  if(l_flag == 1)
	return p;
  else
	return NULL;
}

struct pcb_t *headBlocked(int *semAdd)
{
   void* l_tmp = NULL;
   struct semd_t* l_scan;
   struct pcb_t* l_result;
   int flag = 0;

   clist_foreach(l_scan, &aslh, s_link, l_tmp) 
   {
	if (semAdd==l_scan->s_semAdd){
		flag =1;
		break;
	}
	else if (l_scan->s_semAdd>semAdd){
		break;
	}
   }
   /* if none is found return null. */	
   if ((flag == 0)||(clist_foreach_all(l_scan, &aslh, s_link,l_tmp)))
	return NULL;

   /* if the ASL associated is empty.*/
   if(clist_empty(l_scan->s_link))
	return NULL;

   /* otherwise get the first PCB of this semaphore. */
   l_result = clist_head(l_result, l_scan->s_procq, p_list);
   return l_result;
}

void initASL(void)
{
  static struct semd_t semdTable[MAXPROC];
  int i = 0;

  /* for each element of semdTable, insert it at the end of the list.*/
  for (i = 0; i<MAXPROC; i++){
	clist_enqueue((&semdTable[i]), &semdFree, s_link);	
  }	
}

#endif
