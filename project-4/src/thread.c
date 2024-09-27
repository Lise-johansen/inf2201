/*
 * Implementation of spinlocks, locks and condition variables
 */

#include "common.h"
#include "scheduler.h"
#include "thread.h"
#include "util.h"

/* spinlock functions */

/*
 * xchg exchanges the given value with the value in *addr.  This means
 * that this function will write LOCKED to *addr and return the value
 * that was in *addr.
 *
 * It is done atomically, so if the return value from test_and_set is
 * UNLOCKED, the caller succeeded in getting the lock.  If the return
 * value is LOCKED, then somebody else had the lock and you have to
 * try again.
 */
static int test_and_set(int *addr)
{
	int val = LOCKED;

	asm volatile("xchg (%%eax), %%ebx" : "=b"(val) : "0"(val), "a"(addr));
	return val;
}

void spinlock_init(int *s)
{
	*s = UNLOCKED;
}

/*
 * Wait until lock is free, then acquire lock. Note that this isn't
 * strictly a spinlock, as it yields while waiting to acquire the
 * lock. This implementation is thus more "generous" towards other
 * processes, but might also generate higher latencies on acquiring
 * the lock.
 */
void spinlock_acquire(int *s)
{
	while (test_and_set(s) != UNLOCKED)
		yield();
}

/*
 * Releasing a spinlock is pretty easy, since it only consists of
 * writing UNLOCKED to the address. This does not have to be done
 * atomically.
 */
void spinlock_release(int *s)
{
	ASSERT2(*s == LOCKED, "spinlock should be locked");
	*s = UNLOCKED;
}

/*
 * All thread function is a reuse from my own Project 2: Preemptive Scheduling
 * (13.03.2023) Modiyfied to fit this project, uses spinlock instead of
 * enter/exit critical.
 * Implementasion is the same for all functions: when
 * entering a critical section just call on acquier a spinlock. When leaving a
 * critical section just release the spinlock.
 * But the block function releses the spinlock itself. So when using block, do
 * not relese the spinlock or release it in an else statement.
 */

/* lock functions  */
void lock_init(lock_t *l)
{
	l->waiting = NULL;
	l->status = UNLOCKED;
	spinlock_init(&l->spinlock); // Initialize the spinlock that the lock uses.
}

void lock_acquire(lock_t *l)
{
	/*
	 * The currently running takes the lock and sets the status to locked,
	 * indicating that the lock is not available.
	 * The next therad that arrives are placed in the waiting queue
	 */

	spinlock_acquire(&l->spinlock);

	if (l->status == LOCKED) {
		block(&l->waiting, &l->spinlock);
	} else {
		l->status = LOCKED;
		spinlock_release(&l->spinlock);
	}
}


void lock_release(lock_t *l)
{
	/*
	 * If the waiting queue is not empty, just unlocked on the waiting
	 * queue. Otherwise, set the status to unlocked, since there is no
	 * thread waiting in the queue.
	 */

	spinlock_acquire(&l->spinlock);

	if (l->waiting != NULL) {
		unblock(&l->waiting);
	} else {
		l->status = UNLOCKED;
	}
	spinlock_release(&l->spinlock);
}

/* condition functions */
void condition_init(condition_t *c)
{
	c->waiting = NULL;
	spinlock_init(&c->spinlock); // Initialize the spinlock that the condition uses.
}

/*
 * unlock m and block the thread (enqued on c), when unblocked acquire
 * lock m
 */
void condition_wait(lock_t *m, condition_t *c)
{
	/*
	 * The currently running thread releases the lock and is placed in the
	 * waiting queue. The next thread that arrives takes the lock and
	 * continues execution.
	 */

	spinlock_acquire(&c->spinlock);

	// release the lock m
	lock_release(m);

	// Block the current c in waiting queue
	block(&c->waiting, &c->spinlock);

	// re-acquire the lock m again after c is unblocked
	lock_acquire(m); // uses spinlock to lock m.
}

/* unblock first thread enqued on c */
void condition_signal(condition_t *c)
{
	spinlock_acquire(&c->spinlock);

	/*
	 * If the waiting queue is not empty, unblock the first thread on
	 * waiting queue
	 */
	if (c->waiting != NULL) {
		unblock(&c->waiting);
	}
	spinlock_release(&c->spinlock);
}

/* unblock all threads enqued on c */
void condition_broadcast(condition_t *c)
{
	spinlock_acquire(&c->spinlock);

	/*while there are something in the waiting queue*/
	while (c->waiting != NULL) {

		/*unblock all threads on the waiting queue */
		unblock(&c->waiting);
	}
	spinlock_release(&c->spinlock);
}
