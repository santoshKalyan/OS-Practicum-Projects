Team members: Santoshkalyan Rayadhurgam  and  Atheendra P Tarun 


Project 2: Preemptive Multitasking
Overview
Your task in this project is to extend your non-preemptive user-level threads package with preemption.

In the previous project, you built a non-preemptive thread package and assumed that applications will cooperate with each other and voluntarily yield the CPU frequently. In general, your applications will not be well-behaved, cooperating processes that routinely yield the processor on their own. You need to guard against processes that want to monopolize the CPU by forcibly preempting them and switching to another process.

We have provided you with some code that emulates clock interrupts. The interrupts arrive on the stack of the currently executing thread, just like they do on native hardware. You get to write a clock handler that is supposed to do the right thing in response to the interrupt.




Things to know: Following are the test files we used to test our implementation.

From Makefile:
#MAIN = testMultilevelQueue
#MAIN = testAlarm
#MAIN = test_Mqueue_Scheduling

Scheduling happens in minithread_stop(). This function is called from minithread_unlock_and_stop(),final_proc() and minithread_yield(). We know that, usage of minithread_stop is deprecated but we felt, minithread_stop() is necessary to maintain modularity and it also helps in  code readability.

We have overwritten earlier RERIOD with <91>#define PERIOD (500*MILLISECOND)<92> to have better readabilibty of the output screen. We have tested with 50*MILLISECOND as well.

We have used a dedicated cleanup thread.


