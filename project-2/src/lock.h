/*  lock.h
 */

#ifndef LOCK_H
#define LOCK_H

// Includes
#include "kernel.h"

// Constants
enum
{
	LOCKED = 1,
	UNLOCKED = 0
};

// Typedefs
typedef struct
{
	int status;
	int *array[];

} lock_t;

//  Prototypes
/* initialize the block*/
void lock_init(lock_t *);
/*block*/
void lock_acquire(lock_t *);
/*Unblock*/
void lock_release(lock_t *);

#endif
