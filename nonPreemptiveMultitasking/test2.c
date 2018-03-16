/* test2.c

   Spawn a three threads.
*/

#include "minithread.h"

#include <stdio.h>
#include <stdlib.h>


int thread3(int* arg) {
  printf("\nThread 3.\n");

  return 0;
}

int thread2(int* arg) {
  minithread_t thread = minithread_fork(thread3, NULL);
  printf("\nThread 2.\n");
  minithread_yield();

  return 0;
}

int thread1(int* arg) {
  minithread_t thread = minithread_fork(thread2, NULL);
  printf("\nThread 1.\n");
  minithread_yield();
  printf("\nThread 1. -- After First Yield\n");
  minithread_yield();
  printf("\nThread 1. -- After Second Yield\n");
  
  return 0;
}

main() {
  minithread_system_initialize(thread1, NULL);
}
