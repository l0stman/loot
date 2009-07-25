#include "loot.h"

int inter = 1;	/* interactive mode */

/* Safe malloc. */
void *
smalloc(size_t size)
{
  void *vp;

  if ((vp = malloc(size)) == NULL)
	err_sys("Not enough memory");
  return vp;
}

/* Safe calloc */
void *
scalloc(size_t num, size_t size)
{
  void *vp;

  if ((vp = calloc(num, size)) == NULL)
	err_sys("Not enough memory");
  return vp;
}

/* Safe realloc */
void *
srealloc(void *ptr, size_t size)
{
  void *vp;

  if ((vp = realloc(ptr, size)) == NULL)
	err_sys("Not enough memory");
  return vp;
}

/* Safe strdup */
char *
sstrdup(const char *s)
{
  char *d;

  d = smalloc(strlen(s)+1);
  strcpy(d, s);
  return d;
}

/* Safe strndup */
char *
sstrndup(const char *s, size_t n)
{
  char *d;

  d = smalloc(n+1);
  strncpy(d, s, n);
  *(d+n) = '\0';
  return d;
}

/*
 * Return a pointer to a buf structure.
 */
struct buf *
binit(void)
{
  struct buf *bp;

  bp = smalloc(sizeof(*bp));
  bp->buf = smalloc(BUFSIZ);
  bp->len = 0;
  bp->size = 1;
  return bp;
}

/*
 * Write len bytes from s into bp.
 */
void
bwrite(struct buf *bp, char *s, int len)
{
  while (bp->len+len > BUFSIZ*bp->size)
	bp->buf = srealloc(bp->buf, BUFSIZ * ++(bp->size));
  strncpy(bp->buf+bp->len, s, len);
  bp->len += len;
}
