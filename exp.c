#include "loot.h"
#include "exp.h"

const struct exp false = { ATOM, {"#f"} };
const struct exp true = { ATOM, {"#t"} };
struct exp null = { ATOM, {"()"} };

/* Return true if the two expression a and b are atoms
 * and have the same symbols or if they occupy the
 * same memory.
 */
int
iseq(const struct exp *a, const struct exp *b)
{
  if (a->tp != b->tp)
	return 0;
  else if (isatom(a))
	return (strcmp(a->u.sp, b->u.sp) == 0);
  return (void *)a->u.sp == (void *)b->u.sp;
}

/* Return a string representing an atom */
static char *
atmtostr(const struct exp *ep)
{
  return strdup(ep->u.sp);
}

/* Return a string representing a pair */
static char *
pairtostr(const struct exp *ep)
{
  struct buf *bp;
  char *s, *car, *cdr = NULL;

  bp = binit();
  car = tostr(car(ep->u.cp));
  bputc('(', bp);
  bwrite(bp, car, strlen(car));
  
  if (!isnull(cdr(ep->u.cp))) {
	if (ispair(cdr(ep->u.cp)))
	  bputc(' ', bp);
	else
	  bwrite(bp, " . ", 3);  
	cdr = tostr(cdr(ep->u.cp));
	if (ispair(cdr(ep->u.cp)))	/* don't write the parenthesis */
	  bwrite(bp, cdr+1, strlen(cdr)-2);
	else if (!isnull(cdr(ep->u.cp)))
	  bwrite(bp, cdr, strlen(cdr));
  }
  bwrite(bp, ")", 2);
  s = bp->buf;
  free(car);
  free(cdr);
  free(bp);
  return s;
}

/* Return a string representing a procedure */
#define PROCSTR	"#<procedure"
static char *
proctostr(const struct exp *ep)
{
  struct buf *bp;
  char *s;

  bp = binit();
  bwrite(bp, PROCSTR, strlen(PROCSTR));
  if ((s = ep->u.pp->label) != NULL) {
	bputc(':', bp);
	bwrite(bp, s, strlen(s));
  }
  bwrite(bp, ">", 2);
  s = bp->buf;
  free(bp);
  return s;
}
	
/* Return a string representing the expression */
char *
tostr(const struct exp *ep)
{
  if (isatom(ep))
	return atmtostr(ep);
  else if (ispair(ep))
	return pairtostr(ep);
  else if (isproc(ep))
	return proctostr(ep);
  else
	err_quit("tostr: unknown expression");
  return NULL;
}
