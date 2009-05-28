#ifndef LOOT_H
#define LOOT_H

#ifndef __GNUC__
#define __inline__ inline
#endif

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

#define INPR	"USER> "	/* input prompt */
#define OUTPR	"--> "		/* output prompt */

struct buf {	/* adjustable buffer */
  char *buf;
  int	len;	/* length of characters written in buf */
  size_t size;	/* the size of buf is size*BUFSIZ */
};
void bwrite(struct buf *, char *, int);
struct buf *binit(void);

extern int inter;
void *smalloc(size_t size);
void *srealloc(void *, size_t size);

static __inline__ int
issep(c)
{
  return (isspace(c) || c == '(' || c == ';' || c == ')' ||
		  c == '"' || c == '\'' || c == '.');
}

static __inline__ int
isstop(char a, char b)
{
  return a == '"' ? b == '"': issep(b);
}

/*
 * Write the character c in the adjustable buffer.
 */
static __inline__ void
bputc(int c, struct buf *bp)
{
  if (bp->len == BUFSIZ*bp->size)
	bp->buf = srealloc(bp->buf, BUFSIZ * ++(bp->size));
  bp->buf[bp->len++] = c;
}

/*
 * Free the buf structure.
 */
static __inline__ void
bfree(struct buf *bp)
{
  free(bp->buf);
  free(bp);
}

#endif /* !LOOT_H */
