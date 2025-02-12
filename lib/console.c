#include <stdint.h>
#include "inc/stdio.h"
#include "inc/io.h"
#include "inc/kbdreg.h"

/* Keyboard input code */
#define NO          0
#define SHIFT       (1<<0)
#define CTL         (1<<1)
#define ALT         (1<<2)
#define CAPSLOCK    (1<<3)
#define NUMLOCK     (1<<4)
#define SCROLLLOCK  (1<<5)
#define E0ESC       (1<<6)

static uint8_t shift_code[256] = 
{
    [0x1D] = CTL,
    [0x2A] = SHIFT,
    [0x36] = SHIFT,
    [0x38] = ALT,
    [0x9D] = CTL,
    [0xB8] = ALT
};

static uint8_t toggle_code[256] =
{
    [0x3A] = CAPSLOCK,
    [0x45] = NUMLOCK,
    [0x46] = SCROLLLOCK
};

static uint8_t normal_map[256] =
{
    NO,     0x1B,   '1',    '2',    '3',    '4',    '5',    '6',
    '7',    '8',    '9',    '0',    '-',    '=',    '\b',   '\t',
    'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
    'o',    'p',    '[',    ']',    '\n',   NO,     'a',    's',
    'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
    '\'',   '`',     NO,    '\\',   'z',    'x',    'c',    'v',
    'b',    'n',    'm',    ',',    '.',    '/',    NO,     '*',
    NO,     ' ',    NO,     NO,     NO,     NO,     NO,     NO,
    NO,     NO,     NO,     NO,     NO,     NO,     NO,     '7',
    '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',
    '2',    '3',    '0',    '.',    NO,     NO,     NO,     NO,
    [0xC7] = KEY_HOME,                  [0x9C] = '\n',
    [0xB5] = '/',                       [0xC8] = KEY_UP,
    [0xC9] = KEY_PGUP,                  [0xCB] = KEY_LF,
    [0xCD] = KEY_RT,                    [0xCF] = KEY_END,
    [0xD0] = KEY_DN,                    [0xD1] = KEY_PGDN,
    [0xD2] = KEY_INS,                   [0xD3] = KEY_DEL
};

static uint8_t shift_map[256] =
{
    NO,     033,    '!',    '@',    '#',    '$',    '%',     '^',
    '&',    '*',    '(',    ')',    '_',    '+',    '\b',   '\t',
    'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',
    'O',    'P',    '{',    '}',    '\n',   NO,     'A',    'S',
    'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':',
    '"',    '~',    NO,     '|',    'Z',    'X',    'C',    'V',
    'B',    'N',    'M',    '<',    '>',    '?',    NO,     '*',
    NO,     ' ',    NO,     NO,     NO,     NO,     NO,     NO,
    NO,     NO,     NO,     NO,     NO,     NO,     NO,     '7',
    '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',
    '2',    '3',    '0',    '.',    NO,     NO,     NO,     NO,
    [0xC7] = KEY_HOME,                  [0x9C] = '\n',
    [0xB5] = '/',                       [0xC8] = KEY_UP,
    [0xC9] = KEY_PGUP,                  [0xCB] = KEY_LF,
    [0xCD] = KEY_RT,                    [0xCF] = KEY_END,
    [0xD0] = KEY_DN,                    [0xD1] = KEY_PGDN,
    [0xD2] = KEY_INS,                   [0xD3] = KEY_DEL
};

static uint8_t ctl_map[256] =
{};

static uint8_t *char_code[4] =
{
    normal_map,
    shift_map,
    ctl_map,
    ctl_map
};

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
        data = (shift & E0ESC ? data : data & 0x7F);
        shift &= ~(shift_code[data] | E0ESC);
        return 0;
    }
    else if (shift & E0ESC)
    {
        /* last character was an E0 escape; or with 0x80 */
        data |= 0x80;
        shift &= ~E0ESC;
    }

    shift |= shift_code[data];
    shift ^= toggle_code[data];

    c = char_code[shift & (CTL | SHIFT)][data];

    return c;
}

void kbd_intr(void)
{
    cons_intr(kbd_proc_data);
}

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