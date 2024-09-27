/* kernel.h
 *
 * Various definitions used by the kernel and related code.
 */
#ifndef KERNEL_H
#define KERNEL_H

#include "common.h"

/* Cast 0xf00 into pointer to pointer to function returning void
 * ENTRY_POINT is used in syslib.c to declare 'entry_point'
 */
#define ENTRY_POINT ((void (**)())0xf00)

// Constants
enum
{
	/* Number of threads and processes initially started by the kernel. Change
	 * this when adding to or removing elements from the start_addr array.
	 */
	NUM_THREADS = 8,
	NUM_PROCS = 3,
	NUM_TOTAL = (NUM_PROCS + NUM_THREADS),

	SYSCALL_YIELD = 0,
	SYSCALL_EXIT,
	SYSCALL_COUNT,

	//  Stack constants
	STACK_MIN = 0x10000,
	STACK_MAX = 0x20000,
	STACK_OFFSET = 0x0ffc,
	STACK_SIZE = 0x1000
};

// Typedefs

/* Make the stack for process and threads.
 *  Uses for loop to loop through  the number of threads and processes.
 *  Threads comes first in the stack then the processes comes next.
 */
void make_stack(int i);

/* The process control block is used for storing various information about
 *  a thread or process
 */
typedef struct pcb_t
{

	// Stack pointer to user space
	uint32_t *userspace_stackPointer;
	// stack pointer to kernel space
	uint32_t *kernelspace_stackPointer;
	// Buffer space of size 108 for floating point
	char buffer[108];
	// process number, shows the number of particular process
	uint32_t pid;
	// specifies state i.e new, ready, running, waiting or terminated
	uint32_t processState;
	// differentiate betweeen treads and processes
	uint32_t *is_thread;
	// Pointer to memory addres to process and threads
	uint32_t *addr;
	// Pointer, used to make the PCB arrays a ready queue.
	struct pcb_t *next,
		*previous;
} pcb_t;

/*Location for all address to process and threads
 *
 */
void program_location(int i, pcb_t *pcb);

/*Connection between pcb*/
void pcb_conecting(int i, pcb_t *pcb);

// Variables

// The currently running process, and also a pointer to the ready queue
extern pcb_t *current_running;

// Prototypes
void kernel_entry(int fn);

/*  Helper function for kernel_entry, in entry.S. Does the actual work
 *  of executing the specified syscall.
 *  Process use kernel_entry to call function inside kernel.
 */
void kernel_entry_helper(int fn);
#endif
