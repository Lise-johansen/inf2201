/*  kernel.c
 */
#include "common.h"
#include "kernel.h"
#include "scheduler.h"
#include "th.h"
#include "util.h"

// Statically allocate some storage for the pcb's
pcb_t pcb[NUM_TOTAL];
// Ready queue and pointer to currently running process
pcb_t *current_running;

void _start(void)
{

	/* Declare entry_point as pointer to pointer to function returning void
	 * ENTRY_POINT is defined in kernel h as (void(**)()0xf00)
	 */
	void (**entry_point)() = ENTRY_POINT;

	// load address of kernel_entry into memory location 0xf00
	*entry_point = kernel_entry;

	/*iterate through all pcb's*/
	for (int i = 0; i < NUM_TOTAL; i++)
	{
		/*i = current pcb, ID for process*/
		pcb[i].pid = i;

		/*A new peocess*/
		pcb[i].processState = STATUS_FIRST_TIME;

		/*To make a stack, call make_stack function*/
		make_stack(i);
		/*Connect all pcb together by a linked list*/
		pcb_conecting(i, pcb);
		/*Call on program_location function, a conection between pcb and the address of process and threads is made.*/
		program_location(i, pcb);
	}

	/*Currently running points to the first pcb in the array*/
	current_running = &pcb[0];

	clear_screen(0, 0, 80, 25);
	//  clear_screen(0, 0, 80, 25);
	print_str(0, 0, "Hello world, this is your kernel speaking.");

	scheduler_entry();
	while (1)
		;
}

void make_stack(int i)
{
	/*Uses the if else statmate to differentiate between threads and process, the first 7 are threads, the next two are processes in kernel space,
	  and the next two processe are in user space.*/
	if (i < NUM_THREADS)
	{
		// 1 = thread
		pcb[i].is_thread = 1;

		/*Stack address start at STACK_MIN. Size of one stack is: STACK_MIN + (SIZE *i).
		 *The through  is that on iteration 0, the stack starts at STACK_MIN and in iteration two
		 *the STACK_MIN is on STACK_MIN + 1000 --> STACK_MIN + (STACK_SIZE * 1)
		 */
		pcb[i].kernelspace_stackPointer = (int *)(STACK_MIN + (STACK_SIZE * i));
	}
	else
	{
		// 0 = process
		pcb[i].is_thread = 0;

		pcb[i].kernelspace_stackPointer = (int *)(STACK_MIN + (STACK_SIZE * i));

		pcb[i].userspace_stackPointer = (int *)(STACK_MIN + (STACK_SIZE * (i + NUM_PROCS)));
	}
}

void pcb_conecting(int i, pcb_t *pcb)
{
	/*Connect all pcb toghter via the next and previous popinter.*/
	/*Insert in an emoty list.*/
	if (i == 0)
	{
		pcb[i].next = &pcb[i + 1];
		pcb[i].previous = &pcb[NUM_TOTAL - 1];
	}
	/*Inster last*/
	else if (i == NUM_TOTAL - 1)
	{
		// Are on the last pcb, point back to the "head" that are the currently running pcb
		pcb[i].next = &pcb[0];
		// Previous pcb is the the currently runnign pcb -1
		pcb[i].previous = &pcb[i - 1];
	}
	/*Insert in the middle*/
	else
	{
		//  next is pcb +1
		pcb[i].next = &pcb[i + 1];
		// previous is pcb -1
		pcb[i].previous = &pcb[i - 1];
	}
}

void program_location(int i, pcb_t *pcb)
{
	// array with size NUM_TOTAL + 1.
	int *addr[NUM_TOTAL];

	// Addesses to all threads
	addr[0] = (int *)clock_thread;
	addr[1] = (int *)thread2;
	addr[2] = (int *)thread3;
	addr[3] = (int *)mcpi_thread0;
	addr[4] = (int *)mcpi_thread1;
	addr[5] = (int *)mcpi_thread2;
	addr[6] = (int *)mcpi_thread3;
	addr[7] = (int *)extra_thread4;

	// Addres to process 1, process 2 and process 3 (extra credit)
	addr[8] = (int *)0x5000;
	addr[9] = (int *)0x7000;
	addr[10] = (int *)0x9000;

	/*Iterate through  the memory address to all threads and processes. And connect it to the pcb*/
	for (int j = 0; j < NUM_TOTAL; j++)
	{
		pcb[j].addr = addr[j];
	}
}

void kernel_entry_helper(int fn)
{
	if (fn == SYSCALL_YIELD)
	{
		yield();
	}

	else if (fn == SYSCALL_EXIT)
	{
		exit();
	}
}
