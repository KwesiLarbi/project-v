#include "inc/stdarg.h"

/* lib/console.c */
int getchar(void);
void cputchar(int c);

/* lib/printfmt.c */
void printfmt(void (*putch)(int, void*), void *putdat, const char *fmt, ...);
void vprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt, va_list);

/* lib/printf.c */
int cprintf(const char *fmt, ...);
int vcprintf(const char *fmt, va_list);