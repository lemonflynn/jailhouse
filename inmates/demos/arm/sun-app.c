#include <inmate.h>
#include <gic.h>
#include <sun_task.h>
#include <sun_timer.h>

/*10ms for one beat*/
#define BEATS_PER_SEC		100
#define ENTER_CRITICAL()	do{asm volatile("cpsid i\n");sun_os_start=false;}while(0)
#define EXIT_CRITICAL()		do{asm volatile("cpsie i\n");sun_os_start=true;}while(0)

static u64 ticks_per_beat;
static volatile u64 expected_ticks;

/*
 * Enables blinking LED
 * Banana Pi:           register 0x1c2090c, pin 24
 * Orange Pi Zero:      register 0x1c20810, pin 17
 */
static void *led_reg;
static unsigned int led_pin;

long long task0_stack[256] __attribute__((aligned(4*1024)));
long long task1_stack[256];
long long task2_stack[256];
long long task3_stack[256];

void task0(void);
void task1(void);
void task2(void);

/* Timer callback */
int timer1_cb(void * data);

extern volatile bool sun_os_start;
extern bool need_schedule;
extern struct sun_tcb * curr_tcb;
extern struct sun_tcb * next_tcb;

char * msg = "flynn";

static void handle_IRQ(unsigned int irqn)
{
	static u64 min_delta = ~0ULL, max_delta = 0;
	u64 delta;

	if (irqn != TIMER_IRQ)
		return;

	sun_timer_handler();
	delta = timer_get_ticks() - expected_ticks;
	if (delta < min_delta)
		min_delta = delta;
	if (delta > max_delta)
		max_delta = delta;

	/*
	printk("Timer fired, jitter: %6ld ns, min: %6ld ns, max: %6ld ns\n",
	       (long)timer_ticks_to_ns(delta),
	       (long)timer_ticks_to_ns(min_delta),
	       (long)timer_ticks_to_ns(max_delta));
	*/
	/*
	if (led_reg)
		mmio_write32(led_reg, mmio_read32(led_reg) ^ (1 << led_pin));
	*/

	expected_ticks = timer_get_ticks() + ticks_per_beat;
	timer_start(ticks_per_beat);

	/*false is 0*/
	if(!sun_os_start)
        return;

	//printk("get here 1 %s\n", need_schedule?"true":"false");
    /*decrease current time remained*/
    if(curr_tcb->time_left--)
        return;

	//printk("get here 2\n");
    /*current task has run out of time, time to schedule..*/
    curr_tcb->time_left = SLICE_TIME;

    for(next_tcb = curr_tcb->next_sun_tcb; next_tcb->state == PENDING; next_tcb = next_tcb->next_sun_tcb);

    if(curr_tcb != next_tcb) {
		need_schedule = true;	
		/*
		printk("context switch %p to %p  ...curr: 0x%08x next: 0x%08x\n", 
				curr_tcb, next_tcb,
				curr_tcb->stack_base, next_tcb->stack_base);
		*/
	}
}

void inmate_main(void)
{
	sun_timer_init();
	printk("Initializing the GIC...\n");
	gic_setup(handle_IRQ);
	gic_enable_irq(TIMER_IRQ);

	printk("Initializing the timer...\n");
	ticks_per_beat = timer_get_frequency() / BEATS_PER_SEC;
	expected_ticks = timer_get_ticks() + ticks_per_beat;
	timer_start(ticks_per_beat);

	led_reg = (void *)(unsigned long)cmdline_parse_int("led-reg", 0);
	led_pin = cmdline_parse_int("led-pin", 0);

	/*it's not necessary to init task0's stack frame, because it use the same
    stack frame as main function, put it here is just for consistency*/
    create_task("task0", (unsigned int)task0_stack, sizeof(task0_stack), (unsigned int)task0);
    create_task("task1", (unsigned int)task1_stack, sizeof(task1_stack), (unsigned int)task1);
    create_task("task2", (unsigned int)task2_stack, sizeof(task2_stack), (unsigned int)task2);

	sun_timer_malloc(500, timer1_cb, msg);

	printk("start task0\n");
	start_task(0);
	//task0();
	while(1);
	halt();
}

int timer1_cb(void * data)
{
    static unsigned int i;
    sun_timer_malloc(500, timer1_cb, msg);

	ENTER_CRITICAL();
	printk("This is %d times message-----------------------\n", i++);
	EXIT_CRITICAL();

    return 0;
}

void task0(void)
{
	volatile int i,j;
	volatile int cnt = 0;
	int ccpsr=0;

	asm volatile(
		"mrs	%1, cpsr\n"
		:"+r"(ccpsr));

	while(1) {
		ENTER_CRITICAL();
		printk("This is task 0x%08x %d times \n", ccpsr, cnt++);
		EXIT_CRITICAL();
		for(i=0;i<100000;i++)
			for(j=0;j<100;j++)
				asm volatile("nop\n\t");
	}
}

void task1(void)
{
	volatile int i,j;
	volatile int cnt = 0;

	while(1) {
		ENTER_CRITICAL();
		printk("This is task1 %d times \n", cnt++);
		EXIT_CRITICAL();
		for(i=0;i<100000;i++)
			for(j=0;j<100;j++)
				asm volatile("nop\n\t");
	}
}

void task2(void)
{
	volatile int i,j;
	volatile int cnt = 0;

	while(1) {
		ENTER_CRITICAL();
		printk("This is task2 %d times \n", cnt++);
		EXIT_CRITICAL();
		for(i=0;i<100000;i++)
			for(j=0;j<100;j++)
				asm volatile("nop\n\t");
	}
}
