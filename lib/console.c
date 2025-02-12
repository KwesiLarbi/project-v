#include <stdint.h>
#include "inc/stdio.h"
#include "inc/io.h"
#include "inc/kbdreg.h"

#define BUFFSIZE 512

static int kbd_proc_data(void)
{
    int c;
    uint8_t stat, data;
    static uint32_t shift;

    stat = inb(KBSTATP);
    if ((stat & KBS_DIB) == 0) return -1;

    /* ignore data from mouse */
    if (stat & KBS_TERR) return -1;

    data = inb(KBDATAP);

    if (data == 0xE0)
    {
        /* E0 escape character */
        shift |= E0ESC;
        return 0;
    }
    else if (data & 0x80)
    {
        /* key released */
    }
}

void kbd_intr(void)
{
    cons_intr(kbd_proc_data);
}

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

    serial_intr();
    kbd_intr();

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