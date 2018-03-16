Team members:
        Name: Santoshkalyan Rayadhurgam 
        Name: Atheendra P Tarun 

Overview

Your task in this project is to write a non-preemptive user-level threads package. Most operating systems, provide such a lightweight thread abstraction because, as you might recall from the lectures and the reading, user-level threads provide concurrency with very low-overhead. Your task is to write one.

We have provided you with some skeletal code that creates an environment on top of (Windows,OSX,Linux) that closely resembles the environment on top of raw hardware shortly after booting. We use our host OS to provide us with a virtual processor and memory. It also bootstraps our kernel; that is, it loads it into memory and starts executing it at its entry point. Your task is to build high-level abstractions on top of this environment in the same manner the host OS builds its abstractions on top of the hardware.

There are a few distinct components to this project.

First, you will have to write some generic FIFO (first-in, first-out) enqueue and dequeue operations. We will be relying on this queue implementation throughout the rest of the semester, so it's important that the implementation be efficient. Specifically, enqueue and dequeue operations should both work in O(1) time.
Second, you need to define and implement thread control blocks, and the functions that operate on them. We suggest you start with minithread_create and minithread_yield, and go on to implement the scheduler. Once you have those two working, you can come back to implement the rest of the functionality.
Third, you need to implement a scheduler. For this assignment, all you need is a first-come, first-served scheduler. You can assume that your threads will voluntarily give up the CPU, which means that your test programs should make occasional calls to minithread_yield().
Fourth, you need to implement semaphores in order to be able to synchronize multiple threads.
Finally, you need to implement a simple "retail store" application that uses your minithread, queue, and semaphore implementations. The application models customers and employees at an electronics retail store as threads. Your application should make the best possible use of the components you developed in this project to simulate the following:
It is release day for the new PortPhone 5, the latest device running the revolutionary PortOS from Sirer Systems. Customers are flocking to their local retail store to purchase the new phones. Unfortunately, the new shipment of phones has just arrived and employees are struggling to keep up with demand.
There are N employees . Each employee constantly unpacks phones. Each phone has a unique serial number, and the phones are unpacked in increasing serial order.
There are M customers, who are served on a first come, first served basis. When a customer arrives at the store, they immediately receive a new phone if one is available. Otherwise, they wait in line until more phones become available. Ensure, for sanity
When a customer receives a phone, (s)he immediately activates it by printing the unique serial number to stdout.

Things to know:
        In the Retail Store application, We are using a queue of size N (Number of employees) to limit the number of 
        phones opened contiguously. 
        This way, we can demonstrate the working of threads better. 
        If not required, we can set the queue size to M or some arbitrarily high value.

        We have also yielded after each successful phone opening, to bring some variety in the output. 
        If not required, can be removed.
