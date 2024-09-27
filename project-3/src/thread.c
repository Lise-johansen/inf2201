/*
 * Implementation of locks and condition variables
 */

#include "common.h"
#include "interrupt.h"
#include "scheduler.h"
#include "thread.h"
#include "util.h"

void lock_init(lock_t *l)
{
	/*
	 * no need for critical section, it is callers responsibility to
	 * make sure that locks are initialized only once
	 */

	l->status = UNLOCKED;
	l->waiting = NULL;
}

/* Acquire lock without critical section (called within critical section) */
static void lock_acquire_helper(lock_t *l)
{
	/*
	 * The currently running thread takes the lock and sets the status to locked,
	 * indicating that the lock is not available.
	 * The next therad that arrives is then placed in a waiting queue
	 */

	if (l->status == UNLOCKED)
	{
		l->status = LOCKED;
	}
	else
	{
		block(&l->waiting);
	}
}

void lock_acquire(lock_t *l)
{
	/*
	 * When the lock_acquired_helper function is called within a critical section
	 * it prevents other threads from messing with the lock while it is being acquired.
	 * When the critical section is exited, it allows other threads to access the lock again.
	 */

	enter_critical();
	lock_acquire_helper(l);
	leave_critical();
}

void lock_release(lock_t *l)
{
	/*
	 * To releas a lock in 2 situations the waiting queue is empty or not
	 * If the waiting queue is empty, just set the status the unlocked.
	 * Otherwise, set correct status and unblok the thread in waiting list.
	 */

	enter_critical();

	if (l->waiting != NULL)
	{
		unblock(&l->waiting);
	}
	else
	{
		l->status = UNLOCKED;
	}

	leave_critical();
}

/* condition functions */

void condition_init(condition_t *c)
{
	c->waiting = NULL;
}

/*
 * unlock m and block the thread (enqued on c), when unblocked acquire
 * lock m
 * condition not meet, open lock to be acquire the agiain.
 */
void condition_wait(lock_t *m, condition_t *c)
{
	enter_critical();
	// Realse the lock m
	lock_release(m);

	// Block the current c in waiting queue
	block(&c->waiting);

	// re-acquire the lock m again after c is unblocked
	lock_acquire_helper(m);
	leave_critical();
}

/* unblock first thread enqued on c */
void condition_signal(condition_t *c)
{
	/*
	 * Whats the first thread in waiting queue to wake up.
	 * Check if the queue is not empty, if so unblock thread in waiting queue.
	 */

	enter_critical();
	if (c->waiting != NULL)
	{
		unblock(&c->waiting);
	}
	leave_critical();
}

/* unblock all threads enqued on c */
void condition_broadcast(condition_t *c)
{
	/*
	 * Same thought as the above function but was to unblock all thread
	 * in waiting queue as long the waiting queue is no empty.
	 */

	enter_critical();

	// while there are something in the waiting queue
	while (c->waiting != NULL)
	{
		// Unblock the threads or processes
		unblock(&c->waiting);
	}

	leave_critical();
}

/* Semaphore functions. */
void semaphore_init(semaphore_t *s, int value)
{
	s->value = value;
	s->waiting = NULL;
}

void semaphore_up(semaphore_t *s)
{
	/*
	 * When the value is incement by one it indicates that a resource is being released
	 * If the value is less than zero, than a thread is waiting on semaphore
	 * and it is unblock from the queue
	 */

	enter_critical();
	s->value++;

	if (s->value < 0)
	{
		unblock(&(s->waiting));
	}

	leave_critical();
}

void semaphore_down(semaphore_t *s)
{
	/*
	 * Start of by decrementing tha vaule, that represents resources.
	 * If the value is less than zero that there are no more resources avaulable
	 * and the thread is added on the waiting queue.
	 */

	enter_critical();
	s->value--;

	if (s->value < 0)
	{
		block(&(s->waiting));
	}

	leave_critical();
}

/*
 * Barrier functions
 */

/* n = number of threads that waits at the barrier */
void barrier_init(barrier_t *b, int n)
{
	b->waiting = NULL;
	b->count = n;
	b->n_equal = 0;
}

/* Wait at barrier until all n threads reach it */
void barrier_wait(barrier_t *b)
{
	/*
	 * Only unblock all thread is all n thread has reach the barrier.
	 */

	// Start critical section.
	enter_critical();

	// Increment the count of threads that have reach the barrier
	b->n_equal++;

	if (b->n_equal == b->count)
	{
		// Reset the counter for the next barrier call if all thread has reach the barrier
		b->n_equal = 0;

		while (b->waiting != NULL)
		{

			// Unblock all threads in waiting queue as long as waiting queue is not empty.
			unblock(&b->waiting);
		}
	}

	else
	{
		// block the thread, since all hte threads has not yet reach the barrier.
		block(&b->waiting);
	}

	// End critical section
	leave_critical();
}
