#ifndef _CLIST_H
#define _CLIST_H

typedef unsigned int size_t;

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
#define clist_enqueue(elem, clistp, member) \
	if ((clistp)->next == NULL)  { \
		elem->member.next = &(elem->member); (clistp)->next = &(elem->member); }\
	else { \
		elem->member.next = (clistp)->next->next; (clistp)->next->next = &(elem->member); (clistp)->next = &(elem->member);} \
		

/* add the structure pointed to by elem as the first element of a circular list */
/* clistp is the address of the tail pointer (struct clist *) */
/* member is the field of *elem used to link this list */
#define clist_push(elem, clistp, member)\
	if ((clistp)->next == NULL){\
		elem->member.next = &(elem->member); (clistp)->next = &(elem->member); }\
	else \
		elem->member.next = (clistp)->next->next; (clistp)->next->next = &(elem->member);

/* clist_empty returns true in the circular list is empty, false otherwise */
/* clistx is a struct clist */
#define clist_empty(clistx) (((clistx).next)==NULL)

/* return the pointer of the first element of the circular queue.
	 elem is also an argument to retrieve the type of the element */
/* member is the field of *elem used to link this list */
#define clist_head(elem, clistx, member)({  \
	(clistx.next == NULL) ? NULL :\
		container_of(clistx.next->next,typeof(*elem),member);})	

/* return the pointer of the last element of the circular queue.
	 elem is also an argument to retrieve the type of the element */
/* member is the field of *elem used to link this list */
#define clist_tail(elem, clistx, member) ({ \
	(clistx.next == NULL) ? NULL :\
		container_of(clistx.next,typeof(*elem),member);})

/* clist_pop and clist__dequeue are synonyms */
/* delete the first element of the list (this macro does not return any value) */
/* clistp is the address of the tail pointer (struct clist *) */
#define clist_pop(clistp) clist_dequeue(clistp)
#define clist_dequeue(clistp) \
if ((clistp)->next != NULL) \
if ((clistp)->next == (clistp)->next->next) (clistp)->next = NULL; \
else (clistp)->next->next = (clistp)->next->next->next;


/* delete from a circular list the element whose pointer is elem */
/* clistp is the address of the tail pointer (struct clist *) */
/* member is the field of *elem used to link this list */
#define clist_delete(elem, clistp, member)({\
	void *_tmp=NULL; typeof(elem) _scan=NULL; int _flag=1; \
	if ((clistp)->next != NULL){ \
		_scan = container_of((clistp)->next, typeof(*_scan),member); \
		/* only one element */\
		if ((clistp)->next == (clistp)->next->next && _scan==elem ) \
		       {(clistp)->next=NULL; _flag =0; } \
		else{ \
		/* more than one element */\
			_scan = container_of((clistp)->next->next, typeof(*_scan),member); \
	       	/* if the item to be deleted is the first*/\
			if (elem == _scan) { \
				(clistp)->next->next=(clistp)->next->next->next; _flag=0;\
			} \
			else { \
				while (_tmp != (clistp)->next && _flag ) { \
					_tmp=&(_scan->member); \
	 				_scan=container_of(_scan->member.next,typeof(*_scan),member);\
					if (_scan == elem) { \
						if ( _scan == container_of((clistp)->next, typeof(*_scan), member)) (clistp)->next=_tmp;	\
					container_of(_tmp,typeof(*elem),member)->member.next=_scan->member.next; _flag = 0; \
	       				} \
				} \
			} \
		} \
}_flag ? 1:0;}) /*return 0 element find, 1 otherwise*/
	
/* this macro has been designed to be used as a for instruction,
	 the instruction (or block) following clist_foreach will be repeated for each element
	 of the circular list. elem will be assigned to each element */
/* clistp is the address of the tail pointer (struct clist *) */
/* member is the field of *elem used to link this list */
/* tmp is a void * temporary variable */
#define clist_foreach(scan, clistp, member, tmp) \
	tmp = NULL; if ( (clistp)->next != NULL )\
	for(scan = container_of((clistp)->next->next,typeof(*scan),member); tmp!= (clistp)->next; \
		tmp = &(scan->member), scan = container_of(scan->member.next,typeof(*scan),member)) 

/* this macro should be used after the end of a clist_foreach cycle
	 using the same args. it returns false if the cycle terminated by a break,
	 true if it scanned all the elements */
#define clist_foreach_all(scan, clistp, member, tmp) ({\
	(tmp == (*clistp).next ) ? 1 : 0; })  

/* this macro should be used *inside* a clist_foreach loop to delete the
	 current element */
#define clist_foreach_delete(scan, clistp, member, tmp) \
	if (tmp == NULL){	\
		if((clistp)->next==&scan->member) (clistp)->next=NULL; \
 		else (clistp)->next->next = scan->member.next;} \
	else { \
		if ((clistp)->next == &scan->member ) \
			(clistp)->next=(struct clist *)tmp;	\
		(container_of(tmp,typeof(*scan),member))->member.next = scan->member.next; \
	}

/* this macro should be used *inside* a clist_foreach loop to add an element
	before the current one */
#define clist_foreach_add(elem, scan, clistp, member, tmp) \
	if(tmp == NULL){\
		elem->member.next = &(scan->member); (*clistp).next->next = &(elem->member); }\
	else { elem->member.next = &(scan->member); (container_of(tmp,typeof(*scan),member))->member.next = &(elem->member);}
#endif

