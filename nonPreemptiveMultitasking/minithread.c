
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
#include "minithread.h"
#include "queue.h"
#include "synch.h"

#include <assert.h>

/*
 * A minithread should be defined either in this file or in a private
 * header file.  Minithreads have a stack pointer with to make procedure
 * calls, a stackbase which points to the bottom of the procedure
 * call stack, the ability to be enqueueed and dequeued, and any other state
 * that you feel they must have.
 */
#define TESTING 0


static int threadid;
queue_t readyqueue, stopped_queue, deadqueue;


int getnewthreadid()
	{
		return (++threadid);
	}

int final_proc(void *num)
{
minithread_t dying_thread = currentthread;

if(-1 == queue_dequeue(readyqueue,&currentthread)) 
	currentthread = idlethread;

queue_append(deadqueue,dying_thread);
minithread_switch(&dying_thread->stacktop,&currentthread->stacktop);
}
/* minithread functions */

minithread_t
minithread_fork(proc_t proc, arg_t arg) {
	minithread_t newminithread;
	newminithread = minithread_create(proc,arg);
	if(NULL == newminithread)
		return NULL;
	queue_append(readyqueue,newminithread);
	return newminithread;
}

minithread_t
minithread_create(proc_t proc, arg_t arg) {
		minithread_t newminithread;
	// to do check number of thread created
		if( NULL == proc)
			return NULL;
		
		newminithread = (minithread_t)malloc(sizeof(minithread_s));
		if (newminithread == NULL )
			return NULL;
		newminithread->id = getnewthreadid();
		newminithread->proc = proc;
		newminithread->arg = arg;

minithread_allocate_stack(&newminithread->stackbase,&newminithread->stacktop);
minithread_initialize_stack(&newminithread->stacktop,proc,arg,final_proc,NULL); 	
	
return newminithread;
}

minithread_t
minithread_self() {
	return currentthread;
}

int
minithread_id() {
	return currentthread->id;
}

void
minithread_stop() {
	minithread_t tempthread = currentthread;
	if( -1 == queue_dequeue(readyqueue,&currentthread) )
		currentthread = idlethread;

	minithread_switch(&tempthread->stacktop, &currentthread->stacktop);
			
	currentthread = tempthread;
}

void
minithread_start(minithread_t t) {
	if(t == NULL)
	{
		return;
	}
	queue_append(readyqueue,t);
}

void
minithread_yield() {
			minithread_t tempthread;
			tempthread = currentthread;

			if ( 0 != tempthread->id )
				queue_append(readyqueue,tempthread);

			if( -1 == queue_dequeue(readyqueue,&currentthread) )
				currentthread = idlethread;
			
			minithread_switch(&tempthread->stacktop, &currentthread->stacktop);
			
			currentthread = tempthread; //back to where we have started before switch

			//Clean up

			while( -1 != queue_dequeue(deadqueue,&tempthread)) {
				if(NULL != tempthread)	{	
					minithread_free_stack(&tempthread->stacktop);
				}
			}
}

int idle_proc(void *num)
{
	while(1)
			{
				minithread_yield();
			}
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
void
minithread_system_initialize(proc_t mainproc, arg_t mainarg) {
		threadid = -1;  //uniqueue identifier for threads Lets have my idle thread id be 0
		readyqueue = queue_new();  //for new threads
		stopped_queue = queue_new(); //to store old threads before context switch
		deadqueue = queue_new(); //to store finished threads to clean up from within its parent

		idlethread = minithread_create(idle_proc,NULL);  //as a backup thread - does nothing, but thread_yield()
		minithread_fork(mainproc,mainarg);  // new thread for the user procedure

		currentthread = idlethread;
		idlethread->proc(idlethread->arg);  // start the process from here
}
