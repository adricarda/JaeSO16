/* this file contains prototype of  auxiliary functions */
#ifndef _AUX_H
#define _AUX_H

#include <uARMtypes.h>

#define  TRUE 1
#define FALSE 0
#define KERNEL_MODE(field) (((field & STATUS_SYS_MODE) == STATUS_SYS_MODE) ? 1 : 0)

#define DEV_SEM(sem) ((sem == &pc_timer_s) || ((sem >= &dev_s[0][0]) && (sem <= &dev_s[N_DEV_TYPES][N_DEV_PER_IL])))

#define bit(n) (1UL << (n))

int isDev( int *);

void copybit(unsigned *source,unsigned *dest, int start, int end);

void *a_memset(void *s, unsigned char c, int n);

void onHandlerEnter(unsigned int currentTime);

void onHandlerExit(unsigned int currentTime, int flag);

void copyState(state_t *src, state_t *dst);

void memcopy(void *dest, void *src, int n);

#endif
