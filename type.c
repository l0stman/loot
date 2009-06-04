#include "loot.h"
#include "exp.h"
#include "type.h"

/* Test if the expression is a number */
__inline__ int
isnum(exp_t *ep)
{
  char *cp;

  if (!isatom(ep))
	return 0;
  if (*(cp = symp(ep)) == '+' || *cp == '-') {
	if (*(cp+1) == '\0')
	  return 0;
	cp++;
  }
  for (; *cp != '\0' && isdigit(*cp); cp++)
	;
  return (*cp == '\0' ? 1: 0);
}

/* Test if the expression is constant. */
static __inline__ int
iscst(exp_t *ep)
{
  return iseq(ep, &false) || iseq(ep, &true);
}

/* Test if the expression is self-evaluating */
int
isself(exp_t *ep)
{
  return isnum(ep) || isstr(ep) || iscst(ep);
}
