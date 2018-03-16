#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

int addition(void *num1,void *num2)
{
	return (*(int *)num1) + (*(int *)num2);
}

void 
main(void){
	queue_t myqueue;
	int *item,return_value;
	int (*Fptr)(void *,void *);
	int num1 = 10,num2=-11;

	Fptr = addition;
	//printf("\n\nReturned values from Fptr(&num1,&num2):%d\n",Fptr(&num1,&num2));
	myqueue=queue_new();
	if(myqueue != NULL)
	{
		printf("\nNew Queue is created.\nNo of Elements set to:%d\n",*(int *)myqueue->item);
	}
	else
	{
		printf("\nQueue is not created - look into queue_new()");
	}
	
	item =(int *)malloc(sizeof(int));
	*item = 10;
	queue_prepend(myqueue,(void *)item);

	item =(int *)malloc(sizeof(int));
	*item = 120;
	queue_append(myqueue,(void *)item);

	item =(int *)malloc(sizeof(int));
	*item = 20;
	queue_prepend(myqueue,(void *)item);
	
	queue_delete(myqueue,&item);
	printf("\nqueue contains %d elements\n",queue_length(myqueue));
	
	return_value=queue_delete(myqueue,&item);
	printf("\nqueue_delete - returned Value:%d",return_value);

	queue_iterate(myqueue,addition,&num1);
	queue_iterate(myqueue,addition,&num2);

	
	item =(int *)malloc(sizeof(int));
	*item = 200;

	queue_append(myqueue,(void *)item);
	printf("\nqueue contains %d elements\n",queue_length(myqueue));

	return_value=queue_search(myqueue,item);
	printf("\nqueue_search - returned Value:%d",return_value);

	return_value = queue_dequeue(myqueue,&item);
	printf("\ndequeued:%d(in MAIN) -- return value:%d",*item,return_value);

	return_value = queue_dequeue(myqueue,&item);
	printf("\ndequeued:%d(in MAIN) -- return value:%d",*item,return_value);

	printf("\nqueue contains %d elements\n",queue_length(myqueue));

	return_value = queue_dequeue(myqueue,&item);
	printf("\ndequeued:%d(in MAIN) -- return value:%d",*item,return_value);
	
	return_value = queue_dequeue(myqueue,&item);
	printf("\ndequeued:%d(in MAIN) -- return value:%d",*item,return_value);	

	printf("\nqueue contains %d elements\n",queue_length(myqueue));

	item =(int *)malloc(sizeof(int));
	*item = 120;
	queue_append(myqueue,(void *)item);

	return_value = queue_dequeue(myqueue,&item);
	printf("\ndequeued:%d(in MAIN) -- return value:%d",*item,return_value);	

	printf("\nqueue contains %d elements\n",queue_length(myqueue));

	item =(int *)malloc(sizeof(int));
	*item = 12;
	queue_append(myqueue,(void *)item);

	item =(int *)malloc(sizeof(int));
	*item = 50;
	queue_prepend(myqueue,(void *)item);
	queue_free(myqueue);
/*
*/
	fflush(stdin);
	getchar();
}