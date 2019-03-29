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
#ifndef __SUN_IPC_H
#define __SUN_IPC_H

struct message{
    struct message * next;
    void * data;
    unsigned int size;
};

struct msg_queue{
    struct message * head;
    struct message * tail;
    unsigned int usage;
    unsigned int size;
};

void msg_pool_init(void);
int init_msg_queue(struct msg_queue * queue, unsigned int size);
int push_msg_queue(struct msg_queue * queue, void * data, unsigned int size);
int pop_msg_queue(struct msg_queue * queue, void ** data, unsigned int * size);

#endif
