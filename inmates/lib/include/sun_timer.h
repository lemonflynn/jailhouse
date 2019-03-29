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
#ifndef __SUN_TIMER_H
#define __SUN_TIMER_H

typedef int (* sun_timer_handler_t)(void * data);

struct sun_timer{
    struct sun_timer *    next;
    unsigned int    delta;
    sun_timer_handler_t   handler;
    void *  data;
};

void sun_timer_init(void);
void sun_timer_handler(void);
struct sun_timer * sun_timer_malloc(unsigned int ms, sun_timer_handler_t handler, void * data);
int sun_timer_free(struct sun_timer * timer);

#endif
