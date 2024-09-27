/*
 * Simple counter program, to check the functionality of yield().
 * Each process prints their invocation counter on a separate line
 * Source: this code is from process1.c from this exam.
 */

#include "common.h"
#include "syslib.h"
#include "util.h"

#define ROWS 4
#define COLUMNS 18

static char picture[ROWS][COLUMNS + 1] = {
    "                    ",
    " | __\\_\\_o____/_| ",
};

static void draw(int locx, int locy, int plane);
static void print_counter(void);

void _start(void)
{
    int locx = 50, locy = 1;

    while (1)
    {
        print_counter();
        /* erase plane */
        draw(locx, locy, FALSE);
        locx -= 1;
        if (locx < -20)
        {
            locx = 80;
        }
        /* draw plane */
        draw(locx, locy, TRUE);
        print_counter();
        delay(DELAY_VAL);
        yield();
    }
}

/* print counter */
static void print_counter(void)
{
    static int counter = 0;

    print_str(17, 0, "Process 3 (Plane2)     : ");
    print_int(17, 25, counter++);
}

/* draw plane */
static void draw(int locx, int locy, int plane)
{
    int i, j;

    for (i = 0; i < COLUMNS; i++)
    {
        for (j = 0; j < ROWS; j++)
        {
            /* draw plane */
            if (plane == TRUE)
            {
                print_char(locy + j, locx + i, picture[j][i]);
            }
            /* erase plane */
            else
            {
                print_char(locy + j, locx + i, ' ');
            }
        }
    }
}
