#include "loot.h"
#include "exp.h"

const exp_t false = { ATOM, {"#f"} };
const exp_t true = { ATOM, {"#t"} };
exp_t null = { ATOM, {"()"} };

/* Return true if the two expressions have the same
 * symbols or if they occupy the same memory.
 */
int
iseq(const exp_t *a, const exp_t *b)
{
  if (type(a) != type(b))
	return 0;
  else if (isatom(a))
	return (strcmp(symp(a), symp(b)) == 0);
  return (void *)symp(a) == (void *)symp(b);
}

/* Return a string representing an atom */
static char *
atmtostr(const exp_t *ep)
{
  return sstrdup(symp(ep));
}

/* Return a string representing a pair */
static char *
pairtostr(const exp_t *ep)
{
  struct buf *bp;
  char *s, *car, *cdr = NULL;

  bp = binit();
  car = tostr(car(ep));
  bputc('(', bp);
  bwrite(bp, car, strlen(car));
   
  if (!isnull(cdr(ep))) {
	if (ispair(cdr(ep)))
	  bputc(' ', bp);
	else
	  bwrite(bp, " . ", 3);  
	cdr = tostr(cdr(ep));
	if (ispair(cdr(ep)))	/* don't write the parenthesis */
	  bwrite(bp, cdr+1, strlen(cdr)-2);
	else if (!isnull(cdr(ep)))
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
proctostr(const exp_t *ep)
{
  struct buf *bp;
  char *s;

  bp = binit();
  bwrite(bp, PROCSTR, strlen(PROCSTR));
  if ((s = procp(ep)->label) != NULL) {
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
tostr(const exp_t *ep)
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

/* Return true if the expression is a null-terminated pair */
int
islist(const exp_t *ep)
{
  while (ispair(ep))
	ep = cdr(ep);
  return isnull(ep);
}
