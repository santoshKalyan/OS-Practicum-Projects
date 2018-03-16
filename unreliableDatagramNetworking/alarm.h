#ifndef __ALARM_H_
#define __ALARM_H_

#include "queue.h"
#include "synch.h"
/*
 * This is the alarm interface. You should implement the functions for these
 * prototypes, though you may have to modify some other files to do so.
 */


struct alarm {
	int id;
	int delay;
	void (*func)(void*);
	void *arg;
};
typedef struct alarm *alarm_t;
typedef struct alarm alarm_s;

int alarm_id;

queue_t alarm_queue;
semaphore_t sema_alarm;

/* register an alarm to go off in "delay" milliseconds, call func(arg) */
int register_alarm(int delay, void (*func)(void*), void *arg);

void deregister_alarm(int alarmid);

#endif
