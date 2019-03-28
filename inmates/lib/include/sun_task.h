/*
 * sun operating system
 *
 * Copyright (c) Siemens AG, 2018, 2019
 *
 * Authors:
 *  Francisco flynn<lin.xu@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */
#ifndef __SUN_TASK_H
#define __SUN_TASK_H

#define SLICE_TIME      10  /* 10 * 1ms*/

enum sun_state{
    RUNNING,
    READY,
    PENDING,
};

enum sun_err{
    MSG_POOL_EMPTY  = -4, 
    QUEUE_EMPTY_ERR = -3, 
    QUEUE_FULL_ERR  = -2, 
    PARA_ERR        = -1, 
    NO_ERR          = 0,
};

struct sun_tcb{
    unsigned int            stack_base;
    unsigned int            stack_size;
    unsigned int            stack_bottom;
    unsigned int            time_left;
    struct sun_tcb *    next_sun_tcb;
    enum sun_state      state;
    char *              task_name;
};

int create_task(char * name, unsigned int stack, unsigned int stack_size, unsigned int task);
int start_task(unsigned int task_num);
int suspend_task(unsigned int task_num);
int resume_task(unsigned int task_num);
enum sun_state get_task_state(unsigned int task_num);
void do_scheduler(void);
void dump_regs(void);
void dump_reg(int val);

#endif
