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
#include <sun_task.h>
#include <inmate.h>

#define MAX_TASK_NUM  10
#define HW32_REG(ADDRESS) (*((volatile unsigned long *)(ADDRESS)))

struct sun_tcb tcb_list[MAX_TASK_NUM];
struct sun_tcb * curr_tcb;
struct sun_tcb * next_tcb;
volatile bool sun_os_start = false;
bool need_schedule = false;

static unsigned int task_cnt;

int create_task(char * name, unsigned int stack, unsigned int stack_size, unsigned int task)
{
    if(stack == 0 || stack_size == 0 || task == 0 || task_cnt == MAX_TASK_NUM)
        return PARA_ERR;

    tcb_list[task_cnt].stack_base = stack + stack_size - 16*4;
    tcb_list[task_cnt].stack_size = stack_size;
    tcb_list[task_cnt].stack_bottom = stack;
    tcb_list[task_cnt].state = RUNNING;
    tcb_list[task_cnt].time_left = SLICE_TIME;
    tcb_list[task_cnt].task_name = name;
	printk("%s stack base 0x%08x\n", name, tcb_list[task_cnt].stack_base);
	printk("tcb %p pc 0x%08x\n", &tcb_list[task_cnt], task);
    /* initial pc */
    HW32_REG((tcb_list[task_cnt].stack_base + (14<<2))) = task;
    /* initial xPSR */
    HW32_REG((tcb_list[task_cnt].stack_base + (15<<2))) = 0x00000053;

    tcb_list[0].next_sun_tcb = &tcb_list[task_cnt];

    if(0 != task_cnt)
        tcb_list[task_cnt].next_sun_tcb = &tcb_list[task_cnt-1];

    task_cnt++;

    return NO_ERR;
}

void dump_reg(int val)
{
	printk("\nreg: 0x%08x\n", val);
}

void dump_regs(void)
{
	int tmp;
	printk("-----------dump start---------\n");
#define dump(rx) \
	asm volatile( \
	"str "#rx", %0\n\t" \
	:"=m"(tmp));\
	printk(#rx" 0x%08x\n", tmp)
	
	dump(r14);
	dump(r13);

#undef dump
	printk("-----------dump end---------\n");
}

int start_task(unsigned int task_num)
{
    if(task_num > MAX_TASK_NUM - 1)
        return PARA_ERR;

    curr_tcb = &tcb_list[task_num];
    sun_os_start = true;

	/* swtch sp,
	 *and jump to corresponding task entry
	 */
	asm volatile (
	"mov sp, %0\n"
	"pop {r0-r12, r14}\n"
	"ldr	r6,	[sp], #4\n"
	"ldr	r7,	[sp], #4\n"
	"msr cpsr, r7\n"
	"mov	pc,	r6\n"
	::"r"(curr_tcb->stack_base));

	return 0;
}

int suspend_task(unsigned int task_num)
{
    if(task_num > MAX_TASK_NUM - 1)
        return PARA_ERR;

    tcb_list[task_num].state = PENDING;
    return NO_ERR;
}

int resume_task(unsigned int task_num)
{
    if(task_num > MAX_TASK_NUM - 1)
        return PARA_ERR;

    tcb_list[task_num].state = RUNNING;
    return NO_ERR;
}

enum sun_state get_task_state(unsigned int task_num)
{
    return tcb_list[task_num].state;
}

