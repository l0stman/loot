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
#include <limits.h>
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

typedef struct buf {	/* adjustable buffer */
  char *buf;
  int	len;	/* length of characters written in buf */
  size_t size;	/* the size of buf is size*BUFSIZ */
} buf_t;
void bwrite(buf_t *, char *, int);
buf_t *binit(void);

extern int inter;
extern void *smalloc(size_t);
extern void *scalloc(size_t, size_t);
extern void *srealloc(void *, size_t);
extern char *sstrdup(const char *);
extern char *sstrndup(const char *, size_t);

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
bputc(int c, buf_t *bp)
{
  if (bp->len == BUFSIZ*bp->size)
	bp->buf = srealloc(bp->buf, BUFSIZ * ++(bp->size));
  bp->buf[bp->len++] = c;
}

/*
 * Free the buf structure.
 */
static inline void
bfree(buf_t *bp)
{
  free(bp->buf);
  free(bp);
}

#endif /* !LOOT_H */
