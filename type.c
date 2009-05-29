#include "loot.h"
#include "exp.h"
#include "type.h"

/* Test if the expression is a number */
__inline__ int
isnum(struct exp *ep)
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

/* Test if the expression is a string. */
static __inline__ int
isstr(struct exp *ep)
{
  return isatom(ep) && *symp(ep) == '"';
}

/* Test if the expression is constant. */
static __inline__ int
iscst(struct exp *ep)
{
  return iseq(ep, &false) || iseq(ep, &true) || iseq(ep, &null);
}

/* Test if the expression is self-evaluating */
int
isself(struct exp *ep)
{
  return isnum(ep) || isstr(ep) || iscst(ep);
}
