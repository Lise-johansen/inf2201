/* Simple counter program, to check the functionality of yield().
 * Print time in seconds.
 */

#include "scheduler.h"
#include "th.h"
#include "util.h"

/*
 * This thread runs indefinitely, which means that the
 * scheduler should never run out of processes.
 */
void extra_thread4(void)
{

    while (1)
    {
        print_str(18, 0, " Extra Credits Threads");
        yield();
    }
}
