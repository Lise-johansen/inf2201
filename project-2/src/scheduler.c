/*  scheduler.c
 * source insptiation for dispatch: https://stackoverflow.com/questions/36002592/what-is-the-correct-use-of-multiple-input-and-output-operands-in-extended-gcc-as
 */
#include "common.h"
#include "kernel.h"
#include "scheduler.h"
#include "util.h"

// Call scheduler_entry() to run the 'next' process
void yield(void)
{
    int start = get_timer();

    scheduler_entry();

    int end = get_timer();

    int time = (end - start);
    print_str(16, 0, "Get time says          :");
    print_int(16, 25, time);
}

/* The scheduler picks the next job to run, and removes blocked and exited
 * processes from the ready queue, before it calls dispatch to start the
 * picked process.
 */
void scheduler(void)
{
    if (current_running->processState == STATUS_BLOCKED || current_running->processState == STATUS_EXITED)
    {
        // To update the pcb list we have to removed the current running pcb from the list.
        // Adjust the next and previous pointer, so that the current runnings previous next pcb points to the current runnings next.
        current_running->previous->next = current_running->next;

        // Currently runnings next is not pointing to currently running but to currently runnings previous*/
        current_running->next->previous = current_running->previous;
    }

    /*If currently running is available, set next to currenlty running*/
    current_running = current_running->next;

    // Call dispatch, to jump start the process or thread.
    dispatch();
}

/* dispatch() does not restore gpr's it just pops down the kernel_stack,
 * and returns to whatever called scheduler (which happens to be scheduler_entry,
 * in entry.S).
 */
void dispatch(void)
{
    /*Move pointer to esp, then jump the the memory location to process or thread. Same is done fore both processes and threads.
     * Only differens is that the process move user stack intp esp*/
    /*Only does this when a treads or process have status first time*/

    if ((current_running->is_thread == 1) && (current_running->processState == STATUS_FIRST_TIME))
    {

        current_running->processState = STATUS_READY;

        __asm__ volatile("movl  %0, %%esp\n\t"
                         "jmp *%1\n\t"
                         :
                         : "r"(current_running->kernelspace_stackPointer), "r"(current_running->addr)
                         :

        );
    }
    else if ((current_running->is_thread == 0) && (current_running->processState == STATUS_FIRST_TIME))
    {

        current_running->processState = STATUS_READY;

        __asm__ volatile("movl  %0, %%esp\n\t"
                         "jmp *%1\n\t"
                         :
                         : "r"(current_running->userspace_stackPointer), "r"(current_running->addr)
                         :

        );
    }
}

/* Remove the current_running process from the linked list so it
 * will not be scheduled in the future
 */
void exit(void)
{
    /*Set the current state as exited*/
    current_running->processState = STATUS_EXITED;

    /*call scheduler to pick a new process to run*/
    scheduler_entry();
}

//___________________________________________________________________
// Important, the block and unblock code do not work. So to run the code, just "Ctrl" + "k" + "u"
// Think of the code as high level pseducode.
//___________________________________________________________________

/* 'q' is a pointer to the waiting list where current_running should be
 * inserted
 */
void block(pcb_t **q)
{
    // // Status is block
    // current_running->processState = STATUS_BLOCKED;

    // /* If the waiting queue is emptym just inster (*q) as the current_running*/
    // if ((*q) == NULL)
    // {
    //     (*q) = current_running;
    // }
    // else
    // {
    //     pcb_t *tmp = (*q); // As not to mess with the pointers

    //     while (tmp->next != NULL)
    //     {
    //         tmp = tmp->next; // Iterate through  the queue
    //     }
    //     tmp->next = current_running;
    // }
    // scheduler_entry();
}

/* Must be called within a critical section.
 * Unblocks the first process in the waiting queue (q), (*q) points to the
 * last process.
 */
void unblock(pcb_t **q)
{
    // pcb_t *tmp = (*q);

    // // Ready to run
    // (*q)->processState = STATUS_READY;

    // /*If (*q) is not null,  and as long that (*q) has a previous
    //  *iterate through  the list of threads*/

    // if ((*q) != NULL)
    // {
    //     for (int j = NUM_THREADS; j > 0; j--)
    //     {
    //         if ((*q)->previous != NULL) // ensure that the first elm is not overwriten.
    //         {
    //             q[j] = q[j - 1]; // shift *q left, by one
    //         }
    //     }

    //     /*This lines of code instert a pcb pointet to by (*q)
    //      *into the circular ready queu after the currently running process*/

    //     // Set currently running previous to (*q)
    //     current_running->next->previous = (*q);
    //     // Set next of (*q) to the thread after currently_running
    //     (*q)->next = current_running->next;
    //     // Set the next pointer of currentlyt running to (*q)
    //     current_running->next = (*q);
    //     // Set the previous pointer to currentently_running
    //     (*q)->previous = current_running;
    // }
}