#include <stdio.h>
#include <stdlib.h>
#include "minithread.h"
#include "synch.h"
#include "queue.h"

//Number of employees
#define N 20
//Number of customers
#define M 100
//Max number of phones waiting to be picked up
#define QUEUE_SIZE N

semaphore_t employees;			//for employees
semaphore_t phones;				//for opened phones available for pickup
semaphore_t phone_serial_lock;  //for accessing phone queue and incrementing phone serial
semaphore_t queue_full;			//for phone queue full
semaphore_t queue_empty;		//for phone queue empty

queue_t phone_queue;

int phone_serial = 0;


void openPhone() {
	while(1) {
		semaphore_P(queue_full);
			semaphore_P(employees);				//Obtain an employee
			if( phone_serial < M ) {			//Stop opening phones as soon as you open M (No of customers) phones
				semaphore_V(phones);				//Make the employee open the phone
					semaphore_P(phone_serial_lock);		//Lock the queue operation
						queue_append(phone_queue, ++phone_serial);
						printf("Employee opens phone with serial number %d\n", phone_serial);
					semaphore_V(phone_serial_lock);		//Release the lock
			}
			semaphore_V(employees);				//Release the employee
		semaphore_V(queue_empty);
		minithread_yield();						//Yield to allow a customer to pickup/another employee to open phone
	}
}

void addCustomer(int customer_no) {
	int out = 0;
	int serial;
	semaphore_P(queue_empty);
		semaphore_P(phones);							//Check if there are any phones available in the queue
			semaphore_P(phone_serial_lock);					//Lock the queue so that it can be removed
				if(-1 == queue_dequeue(phone_queue,&serial)) {  //Dequeue the phone
					printf("Error, trying to open a phone that doesn't exist\n");
				} else {
					printf("Customer %d received the phone with serial %d\n", customer_no, serial);
				}
			semaphore_V(phone_serial_lock);
	semaphore_V(queue_full);
}

void schedule() {
	int i = 0;
	while( i < M ) {
		int customer_no = i + 1;
		minithread_fork(addCustomer, customer_no);
		i++;
	}
	i = 0;
	while( i < N ) {
		minithread_fork(openPhone, NULL);
		i++;
	}
	minithread_yield();
}

void main(void) {
	employees = semaphore_create();
	phones = semaphore_create();
	phone_serial_lock = semaphore_create();
	queue_full = semaphore_create();
	queue_empty = semaphore_create();

	

	semaphore_initialize(employees, N);
	semaphore_initialize(phones, 0);
	semaphore_initialize(phone_serial_lock, 1);
	semaphore_initialize(queue_full, QUEUE_SIZE);
	semaphore_initialize(queue_empty, 0);
	phone_queue = queue_new();
  
	minithread_system_initialize(schedule, NULL);  
}