/*
 * Multilevel queue manipulation functions  
 */
#include "multilevel_queue.h"
#include <stdlib.h>
#include <stdio.h>
//extern multilevel_queue_TESTING = 1;
struct multilevel_queue{
	int levels;
	queue_t *queues;
};
/*
 * Returns an empty multilevel queue with number_of_levels levels. On error should return NULL.
 */
multilevel_queue_t multilevel_queue_new(int number_of_levels)
{
	multilevel_queue_t mqueue;
	int i;

	if(number_of_levels <=0)
	return NULL;

	mqueue = (multilevel_queue_t)malloc(sizeof(multilevel_queue_s));
	if(mqueue != NULL)
		{
			mqueue->levels = number_of_levels;
			mqueue->queues = (queue_t)malloc(number_of_levels * sizeof(queue_t));
			if(mqueue->queues != NULL)
				{
					for(i=0;i<number_of_levels;i++)
					{
						mqueue->queues[i] = queue_new();
						if(multilevel_queue_TESTING)
						{
							printf("\nmqueue:%d level queue -- ",i);
							if(mqueue->queues[i] != NULL) printf("Created.");
							else printf(" Problem with creating the queue.");
						}
					}
				}
		}
return mqueue;
}

/*
 * Appends an void* to the multilevel queue at the specified level. Return 0 (success) or -1 (failure).
 */
int multilevel_queue_enqueue(multilevel_queue_t mqueue, int level, void* item)
{
	if(mqueue->levels <= level || level < 0)
	return -1;
	
	if(multilevel_queue_TESTING)
		printf("\nmqueue:Appending %d element to %d level queue",*(int *)item,level);
	return queue_append(mqueue->queues[level],item);
}

/*
 * Dequeue and return the first void* from the multilevel queue starting at the specified level. 
 * Levels wrap around so as long as there is something in the multilevel queue an item should be returned.
 * Return the level that the item was located on and that item if the multilevel queue is nonempty,
 * or -1 (failure) and NULL if queue is empty.
 */
int multilevel_queue_dequeue(multilevel_queue_t mqueue, int level, void** item)
{
	int i,temp,count;
	if(mqueue->levels <= level || level < 0)
	return -1;

	i=level;
	for(count=0;count<mqueue->levels;count++)
	{
		temp = queue_dequeue(mqueue->queues[i],item);
		if(temp == 0) 
		{
			if(multilevel_queue_TESTING)
			{
				printf("\nmqueue:Dequeued %d element from %d level queue",**(int **)item,i);	
			}
		return i;
		}
		i = (i+1)%(mqueue->levels);   
		//i = i-1; if(i<0) i=mqueue->levels-1;
	}
	*item = NULL;
	return -1;
}

/* 
 * Free the queue and return 0 (success) or -1 (failure). Do not free the queue nodes; this is
 * the responsibility of the programmer.
 */
int multilevel_queue_free(multilevel_queue_t mqueue)
{
	int i=0;
	for(i=0;i<mqueue->levels;i++)
	{
		if(multilevel_queue_TESTING)
		{
			printf("\nmqueue:Freeing %d level queue",i);
		}
		free(mqueue->queues[i]);
	}
	if(multilevel_queue_TESTING)
		{
			printf("\nmqueue:Freeing multi-level queue structure");
		}
	free(mqueue);
	return 0;
}
