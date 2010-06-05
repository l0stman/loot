#ifndef ERROR_H
#define ERROR_H

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
        jmp_buf env;
        const char *file;
        int line;
        const excpt_t *exception;
        const char *msg;
} exfram_t;

enum { ENTERED = 0, RAISED, HANDLED };

extern exfram_t *exstack;
void raise(const excpt_t *, const char *, int, const char *, ...);

#define RAISE(e, msg) raise(&(e), filename, linenum, "%s", msg)
#define RERAISE  raise(exfram.exception,exfram.file,exfram.line,"%s",exfram.msg)
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

#define ELSE		if (exflag == ENTERED)			\
                		exstack = exstack->prev;        \
		        } else {				\
                  		exflag = HANDLED;

#define WARN(e)                                 			       \
        if (exflag == ENTERED)                  			       \
                exstack = exstack->prev;        			       \
        } else if (exfram.exception == &(e)) {  			       \
        	if (e.reason)                          		       	       \
                	fprintf(stderr, "%s: %s: %s",                          \
                                progname,                                      \
                                e.reason,                                      \
                                exfram.msg);                                   \
	        else                                               	       \
        	        fprintf(stderr, "%s: exception at 0x%p: %s",	       \
                                progname,                                      \
                                &(e),                                          \
                                exfram.msg);                                   \
        	if (exfram.file && exfram.line > 0)                            \
                	fprintf(stderr, " raised in %s at line %d\n",          \
                                exfram.file,                                   \
                                exfram.line);                                  \
                else                                                           \
                        fprintf(stderr, "\n");                                 \
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

#endif /* !ERROR_H */
