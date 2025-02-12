#include <stdint.h>
#include "inc/stdio.h"
#include "inc/io.h"
#include "inc/kbdreg.h"

static void cons_intr(int (*proc)(void));
static void cons_putc(int c);

/* I/O delay routine */
static void delay(void)
{
    inb(0x84);
    inb(0x84);
    inb(0x84);
    inb(0x84);
}

/* Serial I/O code: used to control and communicate with serial devices */
#define COM1        0x3F8

#define COM_RX      0    /* In: Receive buffer (DLAB=0) */
#define COM_TX      0    /* Out: Transmit buffer (DLAB=0) */
#define COM_DLL     0   /* Out: Divisor Latch Low (DLAB=1) */
#define COM_DLM     1   /* Out: Divisor Latch High (DLAB=1) */
#define COM_IER     1   /* Out: Interrupt Enable Register */
#define     COM_IER_RDI     0x01    /* Enable receiver data interrupt */
#define COM_IIR     2   /* In: Interrupt ID Register */
#define COM_FCR     2   /* Out: FIFO Control Register */
#define COM_LCR     3   /* Out: Line Control Register */
#define     COM_LCR_DLAB    0x80    /* Divisor Latch Access Bit */
#define     COM_LCR_WLEN8   0x03    /* WordLength: 8 bits */
#define COM_MCR     4  /* Out: Modem Control Register */
#define     COM_MCR_RTS     0x02    /* RTS complement */
#define     COM_MCR_DTR     0x01    /* DTR complement */
#define     COM_MCR_OUT2    0x08    /* Out2 complement */
#define COM_LSR     5   /* In: Line Status Register */
#define     COM_LSR_DATA    0x01    /* Data available */
#define     COM_LSR_TXRDY   0x20    /* Transmit buffer available */
#define     COM_LSR_TSRE    0x40    /* Transmitter off */

static bool serial_exists;

static int serial_proc_data(void)
{
    if (!(inb(COM1 + COM_LSR) & COM_LSR_DATA))
        return -1;
    return inb(COM1 + COM_RX);
}

void serial_intr(void)
{
    if (serial_exists)
        cons_intr(serial_proc_data);
}

static void serial_putc(int c)
{
    int i;
    for (i = 0; !(inb(COM1 + COM_LSR) & COM_LSR_TXRDY) && i < 12800; i++)
        delay();
    
    outb(COM1 + COM_TX, c);
}

static void serial_init(void)
{
    /* turn off the FIFO */
    outb(COM1 + COM_FCR, 0);

    /* set speed, requires DLAB latch */
    outb(COM1 + COM_LCR, COM_LCR_DLAB);
    outb(COM1 + COM_DLL, (uint8_t) (115200 / 9600));
    outb(COM1 + COM_DLM);

    /* 8 data bits, 1 stop bit, parity off; turn off DLAB latch */
    outb(COM1 + COM_LCR, COM_LCR_WLEN8 & ~COM_LCR_DLAB);

    /* no modem controls */
    outb(COM1 + COM_MCR, 0);

    /* enable rcv interrupts */
    outb(COM1 + COM_IER, COM_IER_RDI);

    /* clear any pre-existing overrun indications and interrupts */
    /* serial port doesn't exist if COM_LSR returns 0xFF */
    serial_exists = (inb(COM1 + COM_LSR) != 0xFF);
    (void) inb(COM1 + COM_IIR);
    (void) inb(COM1 + COM_RX);
}

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

#define C(x) (x - '@')

static uint8_t ctl_map[256] =
{
    NO,         NO,         NO,         NO,         NO,         NO,         NO,         NO,
    NO,         NO,         NO,         NO,         NO,         NO,         NO,         NO,
    C('Q'),     C('W'),     C('E'),     C('R'),     C('T'),     C('Y'),     C('U'),     C('I'),
    C('O'),     C('P'),     NO,         NO,         '\r',       NO,         C('A'),     C('S'),
    C('D'),     C('F'),     C('G'),     C('H'),     C('J'),     C('K'),     C('L'),     NO,
    NO,         NO,         NO,         C('\\'),    C('Z'),     C('X'),     C('C'),     C('V'),
    C('B'),     C('N'),     C('M'),     NO,         NO,         C('/'),     NO,         NO,
    [0x97] = KEY_HOME,
    [0xB5] = C('/'),                    [0xC8] = KEY_UP,
    [0xC9] = KEY_PGUP,                  [0xCB] = KEY_LF,
    [0xCD] = KEY_RT,                    [0xCF] = KEY_END,
    [0xD0] = KEY_DN,                    [0xD1] = KEY_PGDN,
    [0xD2] = KEY_INS,                   [0xD3] = KEY_DEL
};

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
    if (shift & CAPSLOCK)
    {
        if ('a' <= c && c <= 'z')
        {
            c += 'A' - 'a';
        }
        else if ('A' <= c && c <= 'Z')
        {
            c += 'a' - 'A';
        }
    }

    /* process special keys */
    /* Ctrl-Alt-Del: reboot */
    if (!(~shift & (CTL | ALT)) && c == KEY_DEL)
    {
        cprintf("Rebooting!\n");
        outb(0x92, 0x3);
    }

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

/* output a character to the console */
static void cons_putchar(int c)
{
    serial_putchar(c);
    lpt_putchar(c);
    cga_putchar(c);
}

void cputchar(int c)
{
    cons_putchar(c);
}

int getchar(void)
{
    int c;
    while ((c = cons_getchar()) == 0)
        /* do absolutely nothing */
    return c;
}