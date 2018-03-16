/* testminithread.c

   incremental testing for building minithread.c.
*/

#include "minithread.h"

#include <stdio.h>
#include <stdlib.h>

int hello2()
{
	printf("\nThread3:Start");
	//minithread_fork(hello2,NULL);
	printf("\nThread3:End");
}

int hello()
{
	printf("\nThread2:Start");
minithread_fork(hello2,NULL);
//minithread_fork(hello2,NULL);
	printf("\nThread2:End");
}
int
thread(int* arg) {
	printf("\nThread1:Start");	
minithread_fork(hello,NULL);
minithread_fork(hello2,NULL);
	printf("\nThread1:End");
  return 0;
}

void 
main(void) {
  minithread_system_initialize(thread, NULL);

  fflush(stdin);
  getchar();
}