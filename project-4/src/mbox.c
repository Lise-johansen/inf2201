/*
 * Implementation of the mailbox.
 * Implementation notes:
 *
 * The mailbox is protected with a lock to make sure that only
 * one process is within the queue at any time.
 *
 * It also uses condition variables to signal that more space or
 * more messages are available.
 * In other words, this code can be seen as an example of implementing a
 * producer-consumer problem with a monitor and condition variables.
 *
 * Note that this implementation only allows keys from 0 to 4
 * (key >= 0 and key < MAX_Q).
 *
 * The buffer is a circular array.
 */

#include "common.h"
#include "mbox.h"
#include "thread.h"
#include "util.h"

mbox_t Q[MAX_MBOX];

/*
 * Returns the number of bytes available in the queue
 * Note: Mailboxes with count=0 messages should have head=tail, which
 * means that we return BUFFER_SIZE bytes.
 */
static int space_available(mbox_t *q)
{
	if ((q->tail == q->head) && (q->count != 0)) {
		/* Message in the queue, but no space  */
		return 0;
	}

	if (q->tail > q->head) {
		/* Head has wrapped around  */
		return q->tail - q->head;
	}
	/* Head has a higher index than tail  */
	return q->tail + BUFFER_SIZE - q->head;
}

/* Initialize mailbox system, called by kernel on startup  */
void mbox_init(void)
{
	int i;

	/* 
	* In a for loop iterate through all the mboxs and initialize all values to 0, 
	* and initialize the lock and condition that q (mbox) uses.
	*/
	for (i = 0; i < MAX_MBOX; i++) {
		Q[i].used = 0;
		Q[i].count = 0;
		Q[i].head = 0;
		Q[i].tail = 0;
		lock_init(&Q[i].l);
		condition_init(&Q[i].moreSpace);
		condition_init(&Q[i].moreData);
	}
}

/*
 * Open a mailbox with the key 'key'. Returns a mailbox handle which
 * must be used to identify this mailbox in the following functions
 * (parameter q).
 */
int mbox_open(int key)
{

	/*
	 * If the key is valid (i.e., between 0 and MAX MBOX -1), increment the value of "used" at the index ( i.e. key). 
	 * And then return the key. If the key is not valid, return -1.
	 */
	if (key >= 0 && key < MAX_MBOX) {
		Q[key].used++;
		return key;
	} else {
		return -1;
	}
}

/* Close the mailbox with handle q  */
int mbox_close(int q)
{
	/*
	 * Same as above, but decrement used, at index key. Indicating that the mailbox is no longer in use.
	 * Return the key. And if the key is not valid, return -1.
	 */
	if (q >= 0 && q < MAX_MBOX) {
		Q[q].used--;
		return q;
	} else {
		return -1;
	}
}

/*
 * Get number of messages (count) and number of bytes available in the
 * mailbox buffer (space). Note that the buffer is also used for
 * storing the message headers, which means that a message will take
 * MSG_T_HEADER + m->size bytes in the buffer. (MSG_T_HEADER =
 * sizeof(msg_t header))
 */
int mbox_stat(int q, int *count, int *space)
{
	/*
	 * Connect the *count to the count in the mbox q.
	 * Connect the *space to the space available in a mbox q, by calling on the function space_available.
	 * The return value is 0.
	 */
	*count = Q[q].count;
	*space = space_available(&Q[q]);
	return 0;
}

/* Fetch a message from queue 'q' and store it in 'm'*/
int mbox_recv(int q, msg_t *m)
{
	/*
	 * Goal: Get a message from the mbox q and copy it to m (massage).
	 * Information:
	 * Function uses locks so that operation are an atomic operation.
	 * Modulo operation is used to ensure that the index stays within the bounds
	 * of the buffer.
	 */
	
	lock_acquire(&Q[q].l);

	/*+1 the process holds a lock/condtion variable*/
	current_running->use_lock++;

	/*If empty buffer, consumer wait on more data.*/
	while (Q[q].count == 0) {
		condition_wait(&Q[q].l, &Q[q].moreData);
	}

	/*
	 * A pointer p is created to point to the first byte if m. Both are
	 * char/cast to char.
	 * A for loop iterate through the message and uses the
	 * pointer p to copy each byte of the message in the buffer to m
	 * (representet as p).
	 */
	char *p = (char *)m;

	for (int i = 0; i < MSG_SIZE(m); i++) {
		/* Set that m is equle to the bytes in buffer. And iterate through the message size*/
		p[i] = Q[q].buffer[(Q[q].tail + i) % BUFFER_SIZE];
	}

	/*Update tail by incrementing the "indext" (i.e. tail) by the meassage size*/
	Q[q].tail = (Q[q].tail + MSG_SIZE(m)) % BUFFER_SIZE;

	/*Update count by decrementing*/
	Q[q].count--;

	/* (Consumer) Signal to the producer that there is space for more data*/
	condition_signal(&Q[q].moreSpace);


	lock_release(&Q[q].l);

	/*-1 the process no longer holds a lock/condtion variable*/
	current_running->use_lock--;

	/*Retrun 0 on success*/
	return 0;
}

/* Insert 'm' into the mailbox 'q'  */
int mbox_send(int q, msg_t *m)
{
	lock_acquire(&Q[q].l);

	/*+1 the process holds a lock/condtion variable*/
	current_running->use_lock++;

	/* If buffer is full, producer waits too buffer is not full.*/
	while (space_available(&Q[q]) < MSG_SIZE(m)) {
		condition_wait(&Q[q].l, &Q[q].moreSpace);
	}

	/*Creat a pointer to the first byte of m, and cast to char*/
	char *p = (char *)m;

	/*
	 * Copy each byte of the message from the memory location pointed by m
	 * to the buffer, by iteration through the message.
	 */
	for (int i = 0; i < MSG_SIZE(m); i++) {
		/*m -> buffer*/
		Q[q].buffer[(Q[q].head + i) % BUFFER_SIZE] = p[i];
	}

	/*
	 * Update the head the next available empty slot in the buffer, and
	 * update the count.
	 */
	Q[q].head = (Q[q].head + MSG_SIZE(m)) % BUFFER_SIZE;
	Q[q].count++;

	/* (Producer) Signal to the consumer that there is more data available in the buffer.*/
	condition_signal(&Q[q].moreData);

	lock_release(&Q[q].l);

	/*-1 the process no longer holds a lock/condtion variable*/
	current_running->use_lock--;

	/*Retrun 0 on success*/
	return 0;
}
