#include "loot.h"
#include "exp.h"
#include "env.h"

exp_t *false;
exp_t *true;
exp_t *null;

static struct {
  exp_t **ep;
  const char *name;
} cst[] = {
  {&false, "#f"},
  {&true, "#t"},
  {&null, "()"}
};

/* Initiate the variables and install the constants in the
   environment */
void
instcst(struct env *envp)
{
  int i;

  for (i = 0; i < NELEMS(cst); i++) {
	*cst[i].ep = atom(cst[i].name);
	install(cst[i].name, *cst[i].ep, envp);
  }
}

/* Return true if the two expressions occupy the same memory.*/
int
iseq(const exp_t *a, const exp_t *b)
{
  return symp(a) == symp(b);
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
  buf_t *bp;
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
  buf_t *bp;
  char *s;

  bp = binit();
  bwrite(bp, PROCSTR, strlen(PROCSTR));
  if ((s = (char *)label(ep)) != NULL) {
	bputc(':', bp);
	bwrite(bp, s, strlen(s));
  }
  bwrite(bp, ">", 2);
  s = bp->buf;
  free(bp);
  return s;
}

/* Return a string representing a float */
static char *
ftostr(const exp_t *ep)
{
  char *buf = smalloc(FMAXDIG);

  snprintf(buf, FMAXDIG, "%g", fvalue(ep));
  return buf;
}

/* Return a string representing a rational */
static char *
rtostr(const exp_t *ep)
{
  char *buf = smalloc(strlen(num(ep))+strlen(num(ep))+2);

  sprintf(buf, "%s/%s", num(ep), den(ep));
  return buf;
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
  else if (isfloat(ep))
	return ftostr(ep);
  else if (israt(ep))
	return rtostr(ep);
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
