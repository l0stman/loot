#include <err.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "error.h"

/*
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
void
err_quit(const char *fmt, ...)
{
        char buf[MAXLINE];
        va_list ap;

        va_start(ap, fmt);
        vsnprintf(buf, MAXLINE, fmt, ap);
        errx(EXIT_FAILURE, "%s", buf);
        va_end(ap);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void
err_sys(const char *fmt, ...)
{
        char buf[MAXLINE];
        va_list ap;

        va_start(ap, fmt);
        vsnprintf(buf, MAXLINE, fmt, ap);
        err(EXIT_FAILURE, "%s", buf);
        va_end(ap);
}

exfram_t *exstack = NULL;

/* Raise an exception. */
void raise(const excpt_t *e, const char *file, int line)
{
        exfram_t *p = exstack;

        assert(e);
        if (p == NULL) {
                fprintf(stderr, "Uncaught exception");
                if (e->reason)
                        fprintf(stderr, " %s", e->reason);
                else
                        fprintf(stderr, " at 0x%p", e);
                if (file && line > 0)
                        fprintf(stderr, " raised at %s:%d\n", file, line);
                fprintf(stderr, "\naborting...\n");
                abort();
        }
        p->exception = e;
        p->file = file;
        p->line = line;

        exstack = exstack->prev;
        longjmp(p->env, RAISED);
}
