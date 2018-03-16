#include <stdio.h>

#include "interrupts.h"
#include "alarm.h"
#include "minithread.h"
#include "queue.h"

int sortingCriteria(void *alarm1, void *alarm2) {
	alarm_t a1, a2;
	a1 = (alarm_t)alarm1;
	a2 = (alarm_t)alarm2;

	return (a1->delay > a2->delay);
}

/*
 * insert alarm event into the alarm queue
 * returns an "alarm id", which is an integer that identifies the
 * alarm.
 */
int register_alarm(int delay, void (*func)(void*), void *arg)
{
	alarm_t new_alarm = malloc(sizeof(alarm_s));
	if( NULL == new_alarm ) {
		printf("Unable to allocate memory for alarm\n");
		return -1;
	}
	new_alarm->id = ++alarm_id;
	new_alarm->delay = delay;
	new_alarm->func = func;
	new_alarm->arg = arg;
	semaphore_P(sema_alarm);
		queue_append(alarm_queue, new_alarm);
		queue_sort(alarm_queue, sortingCriteria);
	semaphore_V(sema_alarm);
	return new_alarm->id;
}

int find_alarm_id(void *alarmId, void *item) {
	int aid = *((int *) alarmId);
	alarm_t alarm_obj = (alarm_t) item;
	if (alarm_obj->id == aid){
		return 1;
	}
	return 0;
}


/*
 * delete a given alarm  
 * it is ok to try to delete an alarm that has already executed.
 */
void deregister_alarm(int alarmid)
{
	alarm_t alarm_var;
	semaphore_P(sema_alarm);
		alarm_var = ((struct alarm *) queue_iterate_return(alarm_queue, find_alarm_id, &alarmid));
		if(NULL == alarm_var)
			return;
		else
			queue_delete(alarm_queue, &alarm_var);
	semaphore_V(sema_alarm);
}

