#ifndef _INIT_H
#define _INIT_H

#include <clist.h>
#include <pcb.h>
#include <asl.h>
#include <const.h>
#include <interrupt.h>
#include <exceptions.h>
#include <debug.h>

exhl_area_t int_a;
exhl_area_t tlb_a;
exhl_area_t trp_a;
exhl_area_t sysbp_a;

unsigned int g_process_count, g_soft_block_count;
struct clist g_ready_queue;
struct pcb_t *g_current_process;

int g_dev_s[N_DEV_TYPES][N_DEV_PER_IL];
int g_pc_timer_s;
int g_pseudoClockBlk;
cputime_t g_lastUpdate;
cputime_t g_leftTo100;

#endif
