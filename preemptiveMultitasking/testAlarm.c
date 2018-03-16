/* test3.c

   Ping-pong between threads using semaphores.
*/

#include "minithread.h"
#include "synch.h"

#include <stdio.h>
#include <stdlib.h>


int thread3(int *arg) {
	minithread_sleep_with_timeout(100);
	printf("Thread 3\n");
	return 0;
}

int thread2(int* arg) {
	minithread_t thread = minithread_fork(thread3, NULL);
	printf("Thread 2\n");
	return 0;
}

int thread1(int* arg) {
	minithread_t thread = minithread_fork(thread2, NULL);
	minithread_sleep_with_timeout(2000);
	printf("Thread 1\n");
	return 0;
}

main() {
  minithread_system_initialize(thread1, NULL);
}
