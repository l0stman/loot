#ifndef LOOT_H
#define LOOT_H

#ifndef HAS_INLINE
#ifdef __GNUC__
#define	inline	__inline__
#else	/* !__GNUC__ */
#define inline
#endif	/* __GNUC__ */
#endif	/* !HAS_INLINE */

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

#define INPR	"USER> "	/* input prompt */
#define OUTPR	"--> "		/* output */
#define PREFIX	"HOME"	
#define LOOTRC	".lootrc"
#define LIBNAM	"lib.lt"
#define NELEMS(x)	((sizeof (x))/(sizeof ((x)[0])))

/* maximum number of digits (plus sign) for a 128-bits integer */
#define MAXDIG	39

struct buf {	/* adjustable buffer */
  char *buf;
  int	len;	/* length of characters written in buf */
  size_t size;	/* the size of buf is size*BUFSIZ */
};
void bwrite(struct buf *, char *, int);
struct buf *binit(void);

extern int inter;
void *smalloc(size_t);
void *scalloc(size_t, size_t);
void *srealloc(void *, size_t);
char *sstrdup(char *);
char *sstrndup(char *, size_t);

static inline int
issep(c)
{
  return (isspace(c) || c == '(' || c == ';' || c == ')' ||
		  c == '"' || c == '\'' || c == '.');
}

static inline int
isstop(char a, char b)
{
  return a == '"' ? b == '"': issep(b);
}

/*
 * Write the character c in the adjustable buffer.
 */
static inline void
bputc(int c, struct buf *bp)
{
  if (bp->len == BUFSIZ*bp->size)
	bp->buf = srealloc(bp->buf, BUFSIZ * ++(bp->size));
  bp->buf[bp->len++] = c;
}

/*
 * Free the buf structure.
 */
static inline void
bfree(struct buf *bp)
{
  free(bp->buf);
  free(bp);
}

#endif /* !LOOT_H */
