/*
 * Generic queue implementation.cd
 *
 */
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * Return an empty queue.
 */
queue_t queue_new() {
	queue_t myqueue;
	int *noOfElements;
	myqueue=(queue_t)malloc(sizeof(queue_s));
	noOfElements=(int *)malloc(sizeof(int));
	if(myqueue!=NULL && noOfElements!=NULL)
	{
		myqueue->next=myqueue; //head
		myqueue->prev=myqueue; //tail
		*noOfElements=0;
		myqueue->item=noOfElements;
	}
	return myqueue;
}

/*
 * Prepend a void* to a queue (both specifed as parameters).  Return
 * 0 (success) or -1 (failure).
 */
int 
queue_prepend(queue_t queue, void* item) {
	queue_t head,tail,temp;
	
	head=queue->next;
	tail=queue->prev;
	temp=(queue_t)malloc(sizeof(queue_s));
	if(temp != NULL)
	{
		temp->item=item;
		temp->next=NULL;
		temp->prev=NULL;
		*(int *)queue->item = *(int *)queue->item+1;  
		/*
		shortcut for incrementing the total number of Elements
		int *count;
		count=(int *)queue->item;
		*count = *count + 1;
		*/
		//printf("\nPrepending Element:%d",*(int *)temp->item);
		if(head==queue) //Empty queue
		{
			temp->next=queue;
			temp->prev=queue;

			head=temp;
			tail=temp;
		}
		else  //queue already has at least an element
 		{
			temp->next=head;
			temp->prev=head->prev;

			head->prev=temp;
			head=temp;
		}
		queue->next=head;  //update the header
		queue->prev=tail;
		return 0;
	}
	return -1;
}

/*
 * Append a void* to a queue (both specifed as parameters). Return
 * 0 (success) or -1 (failure). 
 */
int 
queue_append(queue_t queue, void* item) {
	queue_t head,tail,temp;
	
	head=queue->next;
	tail=queue->prev;
	temp=(queue_t)malloc(sizeof(queue_s));
	if(temp != NULL)
	{
		temp->item=item;
		temp->next=NULL;
		temp->prev=NULL;
		*(int *)queue->item = *(int *)queue->item+1;  
		/*
		shortcut for incrementing the total number of Elements
		int *count;
		count=(int *)queue->item;
		*count = *count + 1;
		*/
		if(head==queue) //Empty queue
		{
			//printf("\nInserting First Element:%d",((minithread_t)item)->id);
			temp->next=queue;
			temp->prev=queue;

			head=temp;
			tail=temp;
		}
		else
		{
			//printf("\nInserting Element at the End:%d",((minithread_t)item)->id);
			temp->next=queue;
			temp->prev=tail;

			tail->next=temp;
			tail=temp;
		}

		queue->next=head;  //update the header
		queue->prev=tail;
		return 0;
	}
	return -1;
}

/*
 * Dequeue and return the first void* from the queue or NULL if queue
 * is empty.  Return 0 (success) or -1 (failure).
 */
int queue_dequeue(queue_t queue, void** item) {
	queue_t head,tail,temp;
	
	head=queue->next;
	tail=queue->prev;

	if(head == queue)  // queue is empty
	{
		*item=NULL;
		return -1;
	}
	//if(flag==2){
	//	flag = 3;		
	//	minithread_yield();
	//}

	temp=head;
	head=temp->next;
	head->prev=temp->prev;
	*(int *)queue->item = *(int *)queue->item-1;  
		/*
		shortcut for incrementing the total number of Elements
		int *count;
		count=(int *)queue->item;
		*count = *count - 1;
		*/		
	//printf("\nDequeing %d",*(int *)temp->item);
	*item = (void *)temp->item;
	
	//printf("\nDeleting Element:%d",((minithread_t)*item)->id);
	queue->next=head; //update the header
	queue->prev=tail;
	//Cleanup
	free(temp);
	return 0;
}

/*
 * Iterate the function parameter over each element in the queue.  The
 * additional void* argument is passed to the function as its first
 * argument and the queue element is the second.  Return 0 (success)
 * or -1 (failure).
 */
int queue_iterate(queue_t queue, PFany f, void* item) {
	queue_t head,tail,temp;
	int returnvalue_PFany;
	head = queue->next;  //redundant
	tail = queue->prev;	 //redundant

	temp = head;
	while(temp != queue)
	{
		//printf("\n\nIterating queue element containing:%d",*(int *)temp->item);
		returnvalue_PFany=f(item, temp->item);
		if(returnvalue_PFany == -1) return -1;
		//printf("\nIterating queue element PFany returned:%d",returnvalue_PFany);
		temp=temp->next;
	}
	return 0;
}

/*
 * Free the queue and return 0 (success) or -1 (failure).
 */
int queue_free (queue_t queue) {
	queue_t head,tail,temp;
	head = queue->next;
	tail = queue->prev;

	while(head != queue)
	{
		temp = head;
		head=temp->next;
		head->prev = temp->prev;  //to maintain the integrity. Not actually need as we are deleting all the nodes
		//printf("\nFreeing queue element containing:%d",*(int *)temp->item);
		free(temp);
		*(int *)queue->item = *(int *)queue->item-1;  //no needed, just to conform to the principle
		//used in last printf statement in this function
	}
	if(head != queue) //something is wrong
		return -1;

	//printf("\nFreeing queue header containing:%d",*(int *)head->item);
	free(head);
	return 0;   
}

/*
 * Return the number of items in the queue.
 */
int queue_length(queue_t queue) {
	return *(int *)queue->item;
}


/* 
 * Delete the specified item from the given queue. 
 * Return -1 on error.
 */
int queue_delete(queue_t queue, void** item) {
	queue_t head,tail,temp,prev_temp;

	head=queue->next;
	tail=queue->prev;
	//printf("\nElement to be deleted:%d",**(int **)item);  // **item

	if(head==queue) return -1; //empty queue
	temp=head;
	while(temp != queue && temp->item != *item) temp=temp->next;
	if(temp==queue) return -1; // element not found
	
	prev_temp=temp->prev;
	prev_temp->next=temp->next;
	temp->next->prev=temp->prev;
	
	if(prev_temp==queue) {
		head=prev_temp->next;
		tail=prev_temp->prev;
		
	} // special case of one element
	
	queue->next=head; //update the header - redundant here
	queue->prev=tail;
	//printf("\nDeleted the Element\n");
	*(int *)queue->item = *(int *)queue->item-1;  
		/*
		shortcut for incrementing the total number of Elements
		int *count;
		count=(int *)queue->item;
		*count = *count - 1;
		*/
	//Cleanup
	free(temp);
	return 0;
}

int queue_search(queue_t queue, void* item){
	queue_t head,tail,temp;

	head=queue->next;
	tail=queue->prev;
	//printf("\nElement to be searched:%d",*(int *)item);  // *item

	if(head==queue) return -1; //empty queue
	temp=head;
	while(temp != queue && temp->item != item) temp=temp->next;
	if(temp==queue) return -1; // element not found
	return 0;
}

/*
 * Iterate the function parameter over each element in the queue.  The
 * additional void* argument is passed to the function as its first
 * argument and the queue element is the second.  Return item (success)
 * or NULL (failure).
 */
void *queue_iterate_return(queue_t queue, PFany f, void* item) {
	queue_t head,tail,temp;
	int returnvalue_PFany;
	head = queue->next;  //redundant
	tail = queue->prev;	 //redundant

	temp = head;
	while(temp != queue)
	{
		//printf("\n\nIterating queue element containing:%d",*(int *)temp->item);
		returnvalue_PFany = f(item, temp->item);
		if(returnvalue_PFany == 1) return temp->item;
		if(returnvalue_PFany == -1) return NULL;
		//printf("\nIterating queue element PFany returned:%d",returnvalue_PFany);
		temp=temp->next;
	}
	return NULL;
}

//Sort queue based on a sort function. Return true to swap, false to maintain position.
void queue_sort(queue_t queue, PFany sortFunction) {
	queue_t head,tail, element1, element2;
	void *temp;

	head=queue->next;
	tail=queue->prev;
	//printf("\nElement to be searched:%d",*(int *)item);  // *item

	if(head==queue) return; //empty queue
	temp=head;
	element1 = head;
	while(element1 != queue) {
		element2 = element1->next;
		while(element2 != queue) {
			if(sortFunction(element1->item, element2->item)) {
				temp = element1->item;
				element1->item = element2->item;
				element2->item = temp;
			}
			element2 = element2->next;
		}
		element1 = element1->next;
	}
}