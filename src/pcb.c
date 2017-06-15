#include <pcb.h>
#include <aux.h>
#include <const.h>

static struct clist pcbFree=CLIST_INIT;

void initPcbs(void){ 
	int l_i;
	static struct pcb_t proc_blk[MAXPROC];
	for (l_i=0; l_i< MAXPROC; l_i++){ 
		struct pcb_t *elem = &(proc_blk[l_i]);
		clist_enqueue(elem, &pcbFree, p_list); 	
	}
}

struct pcb_t *allocPcb() {
	if (clist_empty(pcbFree))
		return NULL;
	else 
	{
		struct pcb_t *l_tmp = NULL;
		l_tmp=clist_head(l_tmp ,pcbFree,p_list );
		clist_dequeue(&pcbFree);
		/*reset fields of tmp */
		a_memset(l_tmp, 0, sizeof(struct pcb_t));
		return l_tmp;
	}	
}

void freePcb(struct pcb_t *p) {
	clist_enqueue(p, &pcbFree, p_list);
}

void insertProcQ (struct clist *q, struct pcb_t *p) {
	clist_enqueue(p, q, p_list);
}

struct pcb_t* removeProcQ (struct clist *q) {
	if (clist_empty(*q)) return NULL;
	/* if list pointed to by q is not empty, save first element to return */
	struct clist* l_tmp=q->next->next;
	clist_dequeue(q);
	return container_of(l_tmp, struct pcb_t, p_list); 
} 

struct pcb_t *outProcQ (struct clist *q, struct pcb_t *p) {
	void *l_tmp;
	struct pcb_t *l_scan;
	/* search for p in queue pointed to by q, return p if the search has gone successful */
	clist_foreach(l_scan, q, p_list, l_tmp)
		if (l_scan==p) {
			clist_foreach_delete(l_scan, q, p_list, l_tmp)
			return p;
		};
	/* otherwise return NULL */
	return NULL;
}

struct pcb_t *headProcQ (struct clist *q) {
	return clist_head( (struct pcb_t*)0 , (*q), p_list);
}

int emptyChild(struct pcb_t *p) {
        return (clist_empty((p)->p_children)) ? TRUE : FALSE;
}

void insertChild(struct pcb_t *parent, struct pcb_t *p) {
        (p)->p_parent = parent;
        clist_enqueue(p, &((parent)->p_children), p_siblings);
}

struct pcb_t *removeChild(struct pcb_t *p) {
        if (emptyChild(p)) return NULL;
	struct pcb_t *l_child = clist_head(l_child, (p)->p_children, p_siblings);
        /* l_child point to the first child of p */
	clist_pop(&((p)->p_children));
	(l_child)->p_parent = NULL;
	(l_child)->p_siblings.next = NULL;
        return l_child;
}

struct pcb_t *outChild(struct pcb_t *p) {
        if ((p)->p_parent == NULL) return NULL;
        if (clist_delete(p, &((p)->p_parent->p_children), p_siblings)) return NULL;
        (p)->p_parent = NULL;
        (p)->p_siblings.next = NULL;
        return p;
}

struct pcb_t *childByPid(struct pcb_t *p, pid_t pid) {
	if (emptyChild(p)) return NULL;
	void *tmp;
	struct pcb_t *child;
	clist_foreach(child, &(p->p_children), p_siblings, tmp) 
		if (child->p_pid == pid) return child;
		else if (!emptyChild(child)) return childByPid(child, pid);
	return NULL;
}
