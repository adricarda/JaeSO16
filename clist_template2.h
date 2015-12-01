#ifndef _CLIST_H
#define _CLIST_H

//typedef unsigned int    size_t;

#define container_of(ptr, type, member) ({      \
		    const typeof( ((type *)0)->member ) *__mptr = (ptr);  \
		    (type *)( (char *)__mptr - offsetof(type,member) );})

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/* struct clist definition. It is at the same time the type of the tail
	 pointer of the circular list and the type of field used to link the elements */
struct clist {
	struct clist *next;
};

/* constant used to initialize an empty list */
#define CLIST_INIT {NULL}

/* add the structure pointed to by elem as the last element of a circular list */
/* clistp is the address of the tail pointer (struct clist *) */
/* member is the field of *elem used to link this list */
#define clist_enqueue(elem, clistp, member)

/* add the structure pointed to by elem as the first element of a circular list */
/* clistp is the address of the tail pointer (struct clist *) */
/* member is the field of *elem used to link this list */
#define clist_push(elem, clistp, member) \
 if (clist_empty(*clistp)) { (clistp)->next = &((elem)->member); elem->member.next = &((elem)->member);} \
 else { elem->member.next=(clistp)->next->next ; (clistp)->next->next=&((elem)->member); }


/* clist_empty returns true in the circular list is empty, false otherwise */
/* clistx is a struct clist */
#define clist_empty(clistx) (((clistx).next)==NULL)

/* return the pointer of the first element of the circular queue.
	 elem is also an argument to retrieve the type of the element */
/* member is the field of *elem used to link this list */
#define clist_head(elem, clistx, member) 

/* return the pointer of the last element of the circular queue.
	 elem is also an argument to retrieve the type of the element */
/* member is the field of *elem used to link this list */
#define clist_tail(elem, clistx, member) 

/* clist_pop and clist__dequeue are synonyms */
/* delete the first element of the list (this macro does not return any value) */
/* clistp is the address of the tail pointer (struct clist *) */
#define clist_pop(clistp) clist__dequeue(clistp)
#define clist_dequeue(clistp) 

/* delete from a circular list the element whose pointer is elem */
/* clistp is the address of the tail pointer (struct clist *) */
/* member is the field of *elem used to link this list */
#define clist_delete(elem, clistp, member) ({ \
	clist_empty(*clistp) ? 1 : dlt() ;

static inline int dlt() {
	typeof(clistp) tmp=NULL;
	clist_foreach(scan,clistp,member,tmp)
		if (scan == elem) clist_foreach_delete(scan,clistp,member,tmp);
	if clist_foreach_all(scan,clistp,member,tmp) return 0;
	else return 1;
}

/* this macro has been designed to be used as a for instruction,
	 the instruction (or block) following clist_foreach will be repeated for each element
	 of the circular list. elem will be assigned to each element */
/* clistp is the address of the tail pointer (struct clist *) */
/* member is the field of *elem used to link this list */
/* tmp is a void * temporary variable */

#define clist_foreach(scan, clistp, member, tmp) \
	if ((clistp)->next = NULL) break; \
	tmp=NULL; \
	for (scan = container_of((clistp)->next->next, typeof(*scan),member); \
	tmp != (clistp)->next; \
	tmp = &(scan->member),scan = container_of(scan->member.next, typeof(*scan), member))	

/* this macro should be used after the end of a clist_foreach cycle
	 using the same args. it returns false if the cycle terminated by a break,
	 true if it scanned all the elements */
#define clist_foreach_all(scan, clistp, member, tmp) \
({ tmp ==(clistp)->next ? 1 : 0 ; }) 


/* this macro should be used *inside* a clist_foreach loop to delete the
	 current element */
#define clist_foreach_delete(scan, clistp, member, tmp) \
{	if ((clistp)->next == (clistp)->next->next)	\
		(clistp)->next = NULL;	\
	else if ((&scan->member) == (clistp)->next) {\
	       	tmp = (clistp)->next->next; \
		(clistp)=tmp; }	\
	else	tmp = (&scan->member.next); }

/* this macro should be used *inside* a clist_foreach loop to add an element
	before the current one */
#define clist_foreach_add(elem, scan, clistp, member, tmp) 

#endif
