#ifndef ERR_H
#define ERR_H

#include <assert.h>
#include <setjmp.h>

#define MAXLINE 100     /* Maximum length of an error message. */

extern void err_quit(const char *, ...);
extern void err_sys(const char *, ...);

typedef struct excpt {
        const char *reason;
} excpt_t;

typedef struct exfram {
        struct exfram *prev;
        jmp_buf        env;
        const char    *file;
        unsigned       line;
        unsigned       col;
        const excpt_t *exception;
        const char    *msg;
} exfram_t;

enum { ENTERED = 0, RAISED, HANDLED };

extern exfram_t *exstack;
void raise(const excpt_t *, const char *, unsigned, unsigned ,const char *,...);

#define RAISE(e, ...) raise(&(e), instream->name, instream->line,instream->col,\
                            __VA_ARGS__)
#define RERAISE  raise(exfram.exception, exfram.file, exfram.line, exfram.col, \
                       "%s", exfram.msg)
#define RETURN   switch (exstack = exstack->prev, 0) default: return

#define TRY     do {                            \
                volatile int exflag;            \
                exfram_t exfram;                \
                exfram.prev = exstack;          \
                exstack = &exfram;              \
                exflag = setjmp(exfram.env);    \
                if (exflag == ENTERED) {

#define CATCH(e)        if (exflag == ENTERED)                 \
                                exstack = exstack->prev;       \
                        } else if (exfram.exception == &(e)) { \
                                exflag = HANDLED;

#define WARN(e)                                                 \
        if (exflag == ENTERED)                                  \
                exstack = exstack->prev;                        \
        } else if (exfram.exception == &(e)) {                  \
        if (e.reason)                                           \
                fprintf(stderr, "%s: %s: %s",                   \
                        progname,                               \
                        e.reason,                               \
                        exfram.msg);                            \
        else                                                    \
                fprintf(stderr, "%s: exception at 0x%p: %s",    \
                        progname,                               \
                        (void *)&(e),                           \
                        exfram.msg);                            \
        fprintf(stderr, " raised in %s at %d:%d\n",             \
                exfram.file,                                    \
                exfram.line,                                    \
                exfram.col);                                    \
        exflag = HANDLED;

#define ENDTRY  if (exflag == ENTERED)                  \
                        exstack = exstack->prev;        \
                } if (exflag == RAISED)                 \
                          RERAISE;                      \
        } while (0)

void *xalloc(size_t);
void *xrealloc(void *, size_t);
void xfree(void *);
void xfreeall(void);

#endif /* !ERR_H */
