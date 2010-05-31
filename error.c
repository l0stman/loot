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

exfram_t *exstack = NULL;       /* Reinitialize when loading a new file. */

/* Raise an exception. */
void raise(const excpt_t *e, const char *file, int line, const char *msg)
{
        exfram_t *p = exstack;

        assert(e);
        if (p == NULL) {
                fprintf(stderr, "Uncaught exception");
                if (e->reason)
                        fprintf(stderr, " %s: %s", e->reason, msg);
                else
                        fprintf(stderr, " at 0x%p: %s", e, msg);
                if (file && line > 0)
                        fprintf(stderr, " raised in %s at line %d", file, line);
                fprintf(stderr, "\naborting...\n");
                abort();
        }
        p->exception = e;
        p->file      = file;
        p->line      = line;
        p->msg       = msg;

        exstack = exstack->prev;
        longjmp(p->env, RAISED);
}
