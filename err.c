#include <err.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "extern.h"
#include "err.h"

/*
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
void
err_quit(const char *fmt, ...)
{
        static char buf[MAXLINE];
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
        static char buf[MAXLINE];
        va_list ap;

        va_start(ap, fmt);
        vsnprintf(buf, MAXLINE, fmt, ap);
        err(EXIT_FAILURE, "%s", buf);
        va_end(ap);
}

exfram_t *exstack = NULL;       /* Reinitialize when loading a new file. */

/* Raise an exception. Has the same format as printf. */
void raise(const excpt_t *e,
           const char *file,
           unsigned line,
           unsigned col,
           const char *fmt,
           ...)
{
        exfram_t *p = exstack;
        static char msg[MAXLINE];
        va_list ap;

        assert(e);
        va_start(ap, fmt);
        vsnprintf(msg, sizeof(msg), fmt, ap);
        va_end(ap);
        if (p == NULL) {
                fprintf(stderr, "Uncaught exception");
                if (e->reason)
                        fprintf(stderr, " %s: %s", e->reason, msg);
                else
                        fprintf(stderr, " at 0x%p: %s", (void *)e, msg);
                if (file && line > 0 && col > 0)
                        fprintf(stderr, " raised in %s at %d:%d", file,
                                                                  line,
                                                                  col);
                fprintf(stderr, "\naborting...\n");
                abort();
        }
        p->exception = e;
        p->file      = file;
        p->line      = line;
        p->col       = col;
        p->msg       = msg;

        exstack = exstack->prev;
        longjmp(p->env, RAISED);
}

typedef struct list {
        void *p;
        struct list *next;
} list_t;

static list_t *alloc = NULL; /* List of allocated memories with xalloc. */

/*
 * These memory allocation functions should be used with
 * exceptions avoid memory leaks.
 */
void*
xalloc(size_t nbytes)
{
        list_t *p;

        p = smalloc(nbytes + sizeof(*p));
        p->p = p+1;
        p->next = alloc;
        alloc = p;
        return p->p;
}

void*
xrealloc(void *ptr, size_t nbytes)
{
        list_t *p, *prev;

        assert(alloc);
        if (ptr == alloc->p) {
                alloc = srealloc(alloc, sizeof(*alloc)+nbytes);
                alloc->p = alloc+1;
                return alloc->p;
        }
        for (prev = alloc, p = alloc->next; p; prev = p, p = p->next)
                if (ptr == p->p) {
                        p = srealloc(p, sizeof(*p)+nbytes);
                        p->p = p+1;
                        prev->next = p;
                        return p->p;
                }
        assert(0);
        return NULL;
}

void
xfree(void *ptr)
{
        list_t *p, *prev;

        assert(alloc);
        if (ptr == alloc->p) {
                p = alloc;
                alloc = alloc->next;
                free(p);
                return;
        }
        for (prev = alloc, p = alloc->next; p; prev = p, p = p->next)
                if (ptr == p->p) {
                        prev->next = p->next;
                        free(p);
                        return;
                }
        assert(0);
}

void
xfreeall(void)
{
        list_t *p;

        while (alloc) {
                p = alloc;
                alloc = alloc->next;
                free(p);
        }
}
