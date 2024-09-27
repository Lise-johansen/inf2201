/* lock.c
 *
 * Implementation of locks.
 */

#include "common.h"
#include "lock.h"
#include "scheduler.h"

void lock_init(lock_t *l)
{
    l->status = UNLOCKED;
    l->array[NUM_THREADS] = NULL;
}

void lock_acquire(lock_t *l)
{
    if (l->status == UNLOCKED)
    {
        l->status = LOCKED;
    }
    else
    {
        block(&l->array);
    }
}

void lock_release(lock_t *l)
{

    if (l->array == NULL)
    {
        l->status = UNLOCKED;
    }
    else
    {
        unblock(&l->array);
    }
}
