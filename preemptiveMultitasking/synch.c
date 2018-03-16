#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "synch.h"
#include "queue.h"
#include "minithread.h"

/*
 *	You must implement the procedures and types defined in this interface.
 */

/*
 * Semaphores.
 */
struct semaphore
{
    tas_lock_t lock;
	int count;
	queue_t wait_queue;
};

/*
 * semaphore_t semaphore_create()
 *	Allocate a new semaphore.
 */
semaphore_t semaphore_create()
{
	semaphore_t new_sem = malloc(sizeof(struct semaphore));
	if( NULL == new_sem ) {
		printf("Out of memory while allocating semaphore\n");
		return NULL;
	}
	new_sem->wait_queue = queue_new();
	new_sem->count = 1;
	new_sem->lock = 0;
	return new_sem;
}

/*
 * semaphore_destroy(semaphore_t sem);
 *	Deallocate a semaphore.
 */
void semaphore_destroy(semaphore_t sem)
{
	queue_free(sem->wait_queue);
	free(sem);
}
 
/*
 * semaphore_initialize(semaphore_t sem, int cnt)
 *	initialize the semaphore data structure pointed at by
 *	sem with an initial value cnt.
 */
void semaphore_initialize(semaphore_t sem, int cnt)
{
	sem->count = cnt;
	sem->lock = 0;
}

/*
 * semaphore_P(semaphore_t sem)
 *	P on the sempahore. Your new implementation should use TAS locks.
 */
void semaphore_P(semaphore_t sem)
{
		
	while(1==atomic_test_and_set(&sem->lock))
		minithread_yield();
	//If the semaphore is already being used, then wait
	if(0 > --(sem->count)) {
		//Append to queue and check if successful
		if(queue_append(sem->wait_queue, minithread_self()) < 0) {
			printf("Failed to append to wait queue");
			sem->count++;
			atomic_clear(&sem->lock);
			return;
		}
		minithread_unlock_and_stop(&sem->lock);
	}
	else
		atomic_clear(&sem->lock);
}

/*
 * semaphore_V(semaphore_t sem)
 *	V on the sempahore. Your new implementation should use TAS locks.
 */
void semaphore_V(semaphore_t sem)
{
	//loop until lock is obtained
	while(1==atomic_test_and_set(&sem->lock))
		minithread_yield();
	if(0 >= ++(sem->count)){
		minithread_t new_thread;
		queue_dequeue(sem->wait_queue, &new_thread);
		minithread_start(new_thread);
	}
	//Clear lock after scheduling next thread in wait queue
	atomic_clear(&sem->lock);
}
