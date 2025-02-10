#include "inc/stdio.h"
#include <stdint.h>

#define BUFFSIZE 512

static struct
{
   uint8_t buf[BUFFSIZE];
   uint32_t rpos;
   uint32_t wpos; 
} cons;

/* return the next input character from the console, or 0 if none waiting */
int cons_getchar(void)
{
    int c;

    /* grab next character from the input buffer */
    if (cons.rpos != cons.wpos)
    {
        c = cons.buf[cons.rpos++];
        if (cons.rpos == BUFFSIZE) cons.rpos = 0;
        return c;
    }

    return 0;
}

int getchar(void)
{
    int c;
    while ((c = cons_getc()) == 0)
        /* do absolutely nothing */
    return c;
}