#include <stdint.h>

/* sends data to the IO lanes */
static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

/* receives 8/16/32-bit value from an I/O location. */
static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    __asm__ volatile ( "inb %w1, %b0"
                : "=a"(data)
                : "Nd"(port)
                : "memory");
    return data;
}