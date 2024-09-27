#include "scheduler.h"
#include "screen.h"
#include "thread.h"
#include "util.h"

/*
 * Dining philosphers threads.
 */

enum
{
	THINK_TIME = 9999,
	EAT_TIME = THINK_TIME,
};

volatile int forks_initialized = 0;

/* instance of a lock and condition variable*/
lock_t table;
condition_t phil;

volatile int num_eating = 0;
volatile int scroll_eating = 0;
volatile int caps_eating = 0;

/*Added variables, used to keep track of how much on philosophers eats*/
volatile int nr_num_eating = 0;
volatile int nr_scroll_eating = 0;
volatile int nr_caps_eating = 0;

/* Set to true if status should be printed to screen */
int print_to_screen;

enum
{
	LED_NONE = 0x00,
	LED_SCROLL = 0x01,
	LED_NUM = 0x02,
	LED_CAPS = 0x04,
	LED_ALL = 0x07
};

/* Turns keyboard LEDs on or off according to bitmask.
 *
 * Bitmask is composed of the following three flags:
 * 0x01 -- SCROLL LOCK LED enable flag
 * 0x02 -- NUM LOCK LED enable flag
 * 0x04 -- CAPS LOCK LED enable flag
 *
 * Bitmask = 0x00 thus disables all LEDS, while 0x07
 * enables all LEDS.
 *
 * See http://www.computer-engineering.org/ps2keyboard/
 * and http://forum.osdev.org/viewtopic.php?t=10053
 */
static void update_keyboard_LED(unsigned char bitmask)
{
	/* Make sure that bitmask only contains bits for status LEDs  */
	bitmask &= 0x07;

	/* Wait for keyboard buffer to be empty */
	while (inb(0x64) & 0x02)
		;
	/* Tells the keyboard to update LEDs */
	outb(0x60, 0xed);
	/* Wait for the keyboard to acknowledge LED change message */
	while (inb(0x60) != 0xfa)
		;
	/* Write bitmask to keyboard */
	outb(0x60, bitmask);

	ms_delay(100);
}

static void think_for_a_random_time(void)
{
	volatile int foo;
	int i, n;

	n = rand() % THINK_TIME;
	for (i = 0; i < n; i++)
		if (foo % 2 == 0)
			foo++;
}

static void eat_for_a_random_time(void)
{
	volatile int foo;
	int i, n;

	n = 1;
	for (i = 0; i < n; i++)
		if (foo % 2 == 0)
			foo++;
}

/* Odd philosopher */
void num(void)
{
	print_to_screen = 1;

	/*initialize the table and philosopher*/
	lock_init(&table);
	condition_init(&phil);

	forks_initialized = 1;

	if (print_to_screen)
	{
		print_str(PHIL_LINE, PHIL_COL, "Phil.");
		print_str(PHIL_LINE + 1, PHIL_COL, "Running");
	}

	/*
	 * Idea:
	 * Only when you have acquired the table, do you the right to eat. You do not have the right to eat if you have eaten more than the other.
	 * While you have not eaten more than the other you can eat and have the table.
	 * But when you have eaten more than the other, you are sat on the waiting queue, and do not eat, signal to the other that they can eat, and releases the table.
	 */

	while (1)
	{
		think_for_a_random_time();

		/* Take table*/
		lock_acquire(&table);

		/*check that num have the right to eat*/
		while ((nr_num_eating > nr_caps_eating) || (nr_num_eating > nr_scroll_eating))
		{
			/*Do not have the right to eat, on the waiting queue you go*/
			condition_wait(&table, (&phil));
		}

		print_str(10, 30, "num");

		/* Enable NUM-LOCK LED and disable the others */
		update_keyboard_LED(LED_NUM);

		/*If right to eat, it is registered that you have eat one time*/
		nr_num_eating++;

		/*When eating is 1, you are eating*/
		num_eating = 1;

		/* Two philosopher can not eat at the same time */
		ASSERT(scroll_eating + caps_eating == 0);

		if (print_to_screen)
		{
			print_str(PHIL_LINE, PHIL_COL, "Phil    ");
			print_str(PHIL_LINE + 1, PHIL_COL, "Num    ");
		}

		/* Function decide how long you are eating for*/
		eat_for_a_random_time();

		/*When eating is 0, you are no longer eating*/
		num_eating = 0;

		/*Signal to philosophers on the waiting list, that they can acquire the lock again*/
		condition_signal(&phil);

		/* Release the table, now other can take it*/
		lock_release(&table);

		clear_screen(30, 10, 30 + strlen("num"), 11);
	}
}

void caps(void)
{
	/* Wait until num hasd initialized forks */
	while (forks_initialized == 0)
		yield();

	while (1)
	{
		think_for_a_random_time();

		lock_acquire(&table);

		/*check that caps have the right to eat*/
		while ((nr_caps_eating > nr_scroll_eating) || (nr_caps_eating > nr_num_eating))
		{
			/*Have eaten more then the others, so do not have the right to eat*/
			condition_wait(&table, (&phil));
		}

		print_str(11, 30, "caps");

		/* Enable CAPS-LOCK LED and disable the others */
		update_keyboard_LED(LED_CAPS);

		/*Registered that caps has eaten one more time, and caps is now eating*/
		nr_caps_eating++;
		caps_eating = 1;

		/* Two philosopher can not eat at the same time */
		ASSERT(scroll_eating + num_eating == 0);

		if (print_to_screen)
		{
			print_str(PHIL_LINE, PHIL_COL, "Phil.");
			print_str(PHIL_LINE + 1, PHIL_COL, "Caps   ");
		}

		eat_for_a_random_time();

		/*Caps no longer eating*/
		caps_eating = 0;

		/*Siganl to other that they can acqire table now*/
		condition_signal(&phil);

		/* Releses table */
		lock_release(&table);

		clear_screen(30, 11, 30 + strlen("caps"), 12);
	}
}

void scroll_th(void)
{
	/* Wait until num hasd initialized forks */
	while (forks_initialized == 0)
		yield();

	while (1)
	{
		think_for_a_random_time();

		lock_acquire(&table);

		/*check that scroll have the right to eat*/
		while ((nr_scroll_eating > nr_num_eating) || (nr_scroll_eating > nr_caps_eating))
		{
			/*Eaten to much, so no more eating*/
			condition_wait(&table, (&phil));
		}

		print_str(12, 30, "scroll");

		/* Enable SCROLL-LOCK LED and disable the others */
		update_keyboard_LED(LED_SCROLL);

		/*Registered that scroll has eaten one more time, and scroll is now eating*/
		nr_scroll_eating++;
		scroll_eating = 1;

		/* Two philosopher can not eat at the same time */
		ASSERT(caps_eating + num_eating == 0);

		if (print_to_screen)
		{
			print_str(PHIL_LINE, PHIL_COL, "Phil.");
			print_str(PHIL_LINE + 1, PHIL_COL, "Scroll ");
		}

		eat_for_a_random_time();

		/*Scroll is now longer eating*/
		scroll_eating = 0;

		/* Now other philosophers now that they can acquire the table*/
		condition_signal(&phil);

		/* Release table */
		lock_release(&table);

		clear_screen(30, 12, 30 + strlen("scroll"), 13);
	}
}
