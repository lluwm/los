#ifndef _STDIO_H_
#define _STDIO_H_

#include <include/stdarg.h>

#ifndef NULL
#define NULL	((void *) 0)
#endif
/* kernel/console.c */
void	cputchar(int c);
int		getchar(void);

/* kernel/printf.c */
int		cprintf(const char *fmt, ...);
int		vcprintf(const char *fmt, va_list);

int		snprintf(char *str, int size, const char *fmt, ...);
int		vsnprintf(char *str, int size, const char *fmt, va_list);

/* lib/printfmt.c */
void	vprintfmt(void (*putch)(int, void*), void *putdat, const char *fmt, va_list);		


/* lib/readline.c */
char *	readline(const char *prompt);
#endif
