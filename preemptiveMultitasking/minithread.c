/*
 * minithread.c:
 *	This file provides a few function headers for the procedures that
 *	you are required to implement for the minithread assignment.
 *
 *	EXCEPT WHERE NOTED YOUR IMPLEMENTATION MUST CONFORM TO THE
 *	NAMING AND TYPING OF THESE PROCEDURES.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "interrupts.h"
#include "minithread.h"
#include "queue.h"
#include "synch.h"
#include "alarm.h"
#include "multilevel_queue.h"

#include <assert.h>

/*
 * A minithread should be defined either in this file or in a private
 * header file.  Minithreads have a stack pointer with to make procedure
 * calls, a stackbase which points to the bottom of the procedure
 * call stack, the ability to be enqueueed and dequeued, and any other state
 * that you feel they must have.
 */

#define TESTING 0
#define LEAST_PRIORITY 3  // 0 is highest

static unsigned long threadid;
semaphore_t sema_readyqueue,sema_deadqueue;
queue_t deadqueue;
multilevel_queue_t readyqueue;
minithread_t currentthread, idlethread, cleanup_thread;

int getnewthreadid()
{
	return (++threadid);
}

int final_proc(void *num)
{
	minithread_t dying_thread = currentthread;
	interrupt_level_t l = set_interrupt_level(DISABLED);
	queue_append(deadqueue,dying_thread);
	set_interrupt_level(l);
	semaphore_V(sema_deadqueue);
	minithread_stop();
	return 0;
}

/* minithread functions */

minithread_t minithread_fork(proc_t proc, arg_t arg)
{
	interrupt_level_t l;
	minithread_t newminithread;
	newminithread = minithread_create(proc,arg);
	minithread_start(newminithread);
	return newminithread;
}

minithread_t minithread_create(proc_t proc, arg_t arg)
{
	minithread_t newminithread;
	if( NULL == proc)
			return NULL;	
	newminithread = (minithread_t)malloc(sizeof(minithread_s));
	if (newminithread == NULL )
		return NULL;
	newminithread->id = getnewthreadid();
	newminithread->priority = 0;
	newminithread->start_tick = 0;

	minithread_allocate_stack(&newminithread->stackbase,&newminithread->stacktop);
	minithread_initialize_stack(&newminithread->stacktop,proc,arg,final_proc,NULL); 	
	
	return newminithread;
}

minithread_t minithread_self()
{
	return currentthread;
}

int minithread_id()
{
	return currentthread->id;
}

/* DEPRECATED. Beginning from project 2, you should use minithread_unlock_and_stop() instead
 * of this function.
 */
void minithread_stop()
{
	minithread_t tempthread = currentthread;
	int m_queue_return_value = -1;

	if(schedular_clock_ticks < 80)   //First 80 ticks
		m_queue_return_value = multilevel_queue_dequeue(readyqueue, 0, &currentthread);
	else if(schedular_clock_ticks < 120) //Next 40 ticks
		m_queue_return_value = multilevel_queue_dequeue(readyqueue, 1, &currentthread);
	else if(schedular_clock_ticks < 144) //Next 24 ticks
		m_queue_return_value = multilevel_queue_dequeue(readyqueue, 2, &currentthread);
	else // if(schedular_clock_ticks < 160){ //Next 14 ticks
		m_queue_return_value = multilevel_queue_dequeue(readyqueue, 3, &currentthread);
	
	if(m_queue_return_value == -1){
		currentthread = idlethread;
	}
	else{
		currentthread->start_tick = global_timer; 
	}
	
	if( currentthread == tempthread ) {
		set_interrupt_level(ENABLED);
		return;
	}
	minithread_switch(&tempthread->stacktop, &currentthread->stacktop);
}

void minithread_start(minithread_t t)
{
	interrupt_level_t l;
	if(t == NULL)
		return;
	l = set_interrupt_level(DISABLED);
	multilevel_queue_enqueue(readyqueue, t->priority, t);
	set_interrupt_level(l);
}

int get_quanta(int priority){ 
	if(priority < 0 || priority > 3)
		return 0; 
	return (int)pow(2,priority);
}

void minithread_yield()
{
	interrupt_level_t l;
	minithread_t tempthread;
	tempthread = currentthread;
	l = set_interrupt_level(DISABLED);

	if ( 0 != tempthread->id ){
		multilevel_queue_enqueue(readyqueue, tempthread->priority, tempthread);
	}
	minithread_stop();
	set_interrupt_level(l);
}

int idle_proc(void *num)
{
	while(1)
		minithread_yield();
}

int cleanup_proc(void *arg)
{
    int l;
    minithread_t tempthread;
    while(1){
        semaphore_P(sema_deadqueue);

        l = set_interrupt_level(DISABLED);
        if( -1 != queue_dequeue(deadqueue,&tempthread))
            minithread_free_stack(&tempthread->stacktop);
        set_interrupt_level(l);
    }
}

int alarm_checker(void *time_elapsed, PFany alarm) {
	int t = (*(int *)time_elapsed);
	alarm_t a = (alarm_t) alarm;
	if(t >= a->delay)
		return 1;
	return -1; //Return -1 to stop search
}

/*
 * This is the clock interrupt handling routine.
 * You have to call minithread_clock_init with this
 * function as parameter in minithread_system_initialize
 */
void clock_handler(void* arg)
{
	int quanta;
	alarm_t alarm_var;
	set_interrupt_level(DISABLED);
	global_timer++;
	schedular_clock_ticks = (schedular_clock_ticks + 1) % 160;
	
	//semaphore_P(sema_alarm);
	while(1) {
		alarm_var = ((struct alarm *) queue_iterate_return(alarm_queue, alarm_checker, &global_timer));
		if(NULL != alarm_var) {
			alarm_var->func(alarm_var->arg);
			queue_delete(alarm_queue, &alarm_var);
		}
		else
			break;
	}
	//semaphore_V(sema_alarm);
	quanta = get_quanta(currentthread->priority);
	if(quanta < (global_timer - currentthread->start_tick)){ //this thread took more quanta
		if(currentthread->priority + 1 <= LEAST_PRIORITY){	// LEAST_PRIORITY == 3
			if(mqueue_scheduling_TESTING)
				printf("\nTook more than alloted quanta, decreasing the priority( %d->%d ):Thread Id :%d\n", 
																			currentthread->priority,
																			currentthread->priority+1,
																			currentthread->id);
			currentthread->priority = currentthread->priority + 1;
		}
	minithread_yield();
	}
	if(mqueue_scheduling_TESTING)
		printf("\nThread ID : %d --Allocted Quanta:%d\tIt took:%d", minithread_id(),
																	quanta,
																	(global_timer - currentthread->start_tick));
}

/*
 * Initialization.
 *
 * 	minithread_system_initialize:
 *	 This procedure should be called from your C main procedure
 *	 to turn a single threaded UNIX process into a multithreaded
 *	 program.
 *
 *	 Initialize any private data structures.
 * 	 Create the idle thread.
 *       Fork the thread which should call mainproc(mainarg)
 * 	 Start scheduling.
 *
 */
void minithread_system_initialize(proc_t mainproc, arg_t mainarg)
{
	global_timer = 0;
	threadid = -1;  //uniqueue identifier for threads Lets have my idle thread id be 0
	alarm_id = -1;
	readyqueue = multilevel_queue_new(4);
	deadqueue = queue_new(); //to store finished threads to clean up from within its parent
	alarm_queue = queue_new();
	sema_deadqueue =  semaphore_create();
	semaphore_initialize(sema_deadqueue, 0);

	sema_readyqueue =  semaphore_create();
	semaphore_initialize(sema_readyqueue, 1);

	sema_alarm = semaphore_create();
	semaphore_initialize(sema_alarm, 1);

	idlethread = minithread_create(idle_proc,NULL);  //as a backup thread - does nothing, but thread_yield()
	cleanup_thread = minithread_fork(cleanup_proc,NULL);
	minithread_fork(mainproc,mainarg);  // new thread for the user procedure
	currentthread = idlethread;
	minithread_clock_init(clock_handler);
	idle_proc(NULL);  // start the process from here
}

/*
 * minithread_unlock_and_stop(tas_lock_t* lock)
 *	Atomically release the specified test-and-set lock and
 *	block the calling thread.
 */
void minithread_unlock_and_stop(tas_lock_t* lock)
{
	interrupt_level_t l;
	l = set_interrupt_level(DISABLED);
	if(NULL != lock)
		atomic_clear(lock);
	minithread_stop();
	set_interrupt_level(l);
}

void wakeup(void *sema) {
	semaphore_t s = (semaphore_t) sema;
	semaphore_V(s);
}
/*
 * sleep with timeout in milliseconds
 */
void minithread_sleep_with_timeout(int delay)
{
	interrupt_level_t level;
	semaphore_t s = semaphore_create();
	semaphore_initialize(s, 0);
	level = set_interrupt_level(DISABLED);
	register_alarm(global_timer + (int)((delay * MILLISECOND) / PERIOD), wakeup, (void *)s);
	set_interrupt_level(level);
	semaphore_P(s);
	semaphore_destroy(s);
}
