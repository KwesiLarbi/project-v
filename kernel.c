#include <stdbool.h> /* supports the bool data types (bool, true, false, __bool_true_false_are_defined) */
#include <stddef.h>  /* defines various variable types and macros (ptrdiff_t, size_t, wchar_t) */
#include <stdint.h>  /* provides a set of integer types with specified widths */

/* 
    - #ifdef: used to include a section of code if a certain macro is defined by define 
    - checks if the compiler thinks you are targeting the wrong operating system
    - __linux__ is a macro that indicates that the platform is linux (always predefined)
*/
#ifdef __linux__ 
#error You are not using a cross-compiler, you will most certainly run into trouble
#endif

/*
    - #ifndef: used to include a section of code if a certain macro is not defined by #define
    - this program will only work for the 32-bit ix86 targets
    - defined if a 32-bit x86 instruction set is the target
*/
#ifndef __i386__
#error Needs to be with ix86-elf compiler.
#endif

/* user defined hardware text mode color constants. */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12, 
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    /* 
    fg = 0 decimal/00000000 binary, bg = 7 decimal/00000111 binary
    fg | bg => 0 | 7 = 7 => 7 << 4 = 112 decimal/11100000 binary
    */
    return fg | bg << 4; 
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    /* 
    line 88 example: 
        type casting uc and color to uint16_t type
        uc = ' ' has an ASCII value of 32, so 32 decimal/10000000 binary, color = 112 decimal/11100000 binary
        uc | color => 32/10000000 | 112/11100000 = 112/11100000 => 112 << 8 = 28704 decimal/111000000100000 binary
    */
    return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str)
{
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize(void)
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c)
{
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);

    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;

        if (++terminal_row == VGA_HEIGHT) terminal_row = 0;
    }
}

void terminal_write(const char* data, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char* data)
{
    terminal_write(data, strlen(data));
}

void kernel_main(void)
{
    /* initialize terminal interface */
    terminal_initialize();

    /* TODO: newline support */
    terminal_writestring("Hello, World!");
}

