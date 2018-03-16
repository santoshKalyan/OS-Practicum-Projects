/* test_Mqueue_Scheduling.c

   Ping-pong between threads using semaphores.
*/

#include "minithread.h"
#include "synch.h"

#include <stdio.h>
#include <stdlib.h>

extern mqueue_scheduling_TESTING=1;

semaphore_t sem1;
semaphore_t sem2;
int x = 0;

int thread2(int* arg) {
	long i = 0;
	long j = 0;
  while (x < 20000000) {
    printf("Thread 2, x = %d.\n", x++);
	i = 0;
	while(i++ < 1000000000){
		j = 0;
		//while(j++ < 2);
	}
    semaphore_V(sem1);
    semaphore_P(sem2);
  }

  return 0;
}

int thread1(int* arg) {
  minithread_t thread = minithread_fork(thread2, NULL);
	long i = 0;
	long j = 0;
  while (x < 2000000) {
    printf("Thread 1, x = %d.\n", x++);
    i = 0;
	while(i++ < 1000000000){
		j = 0;
		//while(j++ < 2);
	}

	semaphore_P(sem1);
    semaphore_V(sem2);
  }

  return 0;
}

main() {
  sem1 = semaphore_create();
  semaphore_initialize(sem1, 0);
  sem2 = semaphore_create();
  semaphore_initialize(sem2, 0);
  minithread_system_initialize(thread1, NULL);
}
