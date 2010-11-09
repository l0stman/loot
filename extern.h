#ifndef EXTERN_H
#define EXTERN_H

#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "err.h"

#define INPR      "USER> "      /* input prompt */
#define OUTPR     "--> "        /* output */
#define PREFIX    "HOME"
#define LOOTRC    ".lootrc"
#define LIBNAM    "lib.scm"
#define NELEMS(x) ((sizeof (x))/(sizeof ((x)[0])))

/* maximum number of digits (plus sign) for a 128-bits integer */
#define MAXDIG  39
#define FMAXDIG 2*MAXDIG
#define IMAXDIG 10

#define NEW(p)	((p) = smalloc(sizeof *(p)))

#define BSIZ	32

typedef struct buf {            /* adjustable buffer */
        char   *buf;
        int     len;            /* length of the buffer */
        size_t  size;           /* size of the buffer */
} buf_t;

extern int   linenum;
extern char *filename;
extern const char *progname;

extern void *smalloc(size_t);
extern void *scalloc(size_t, size_t);
extern void *srealloc(void *, size_t);
extern char *sstrdup(const char *);
extern char *sstrndup(const char *, size_t);

static inline int
issep(int c)
{
        return (isspace(c) || c == '(' || c == ';' ||
                c == ')' || c == '"' || c == '\'');
}

/* Resize the buffer to nbytes. */
static inline buf_t *
bresize(buf_t *bp, size_t nbytes)
{
        bp = xrealloc(bp, sizeof(*bp)+nbytes);
        bp->buf = (char *)(bp+1);
        bp->size = nbytes;
        return bp;
}

/* Return a pointer to a buf structure. */
static inline buf_t *
binit(void)
{
        buf_t *bp;

        bp = xalloc(sizeof(*bp)+BSIZ);
        bp->buf = (char *)(bp+1);
        bp->len = 0;
        bp->size = BSIZ;
        return bp;
}

#define bwrite(bp, s, len)	_bwrite(&(bp), s, len)

/* Write len bytes from s into the buffer. */
static inline void
_bwrite(buf_t **p, char *s, int len)
{
        while ((*p)->len+len > (*p)->size)
                *p = bresize(*p, (*p)->size+BSIZ);
        memcpy((*p)->buf+(*p)->len, s, len);
        (*p)->len += len;
}

#define bputc(c, bp)	_bputc(c, &(bp))

/* Write the character c in the adjustable buffer. */
static inline void
_bputc(int c, buf_t **p)
{
        if ((*p)->len == (*p)->size)
                *p = bresize(*p, (*p)->size+BSIZ);
        (*p)->buf[(*p)->len++] = c;
}

/* Free the buf structure. */
static inline void
bfree(buf_t *bp)
{
        xfree(bp);
}

#endif /* !EXTERN_H */
