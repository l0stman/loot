#include "loot.h"
#include "exp.h"
#include "type.h"

/* Test if the expression is an integer */
int
isint(const char *s, int len, int *p)
{
  const char *cp;
  
  if (*(cp = s) == '+' || *cp == '-') {
	if (len == 1)
	  goto fail;
	cp++;
  }
  for (; cp-s < len && isdigit(*cp); cp++)
	;
  if (cp-s == len)
	return 1;
 fail:
  if (p)
	*p = cp-s;
  return 0;
}

/* Test if the expression is a number */
int
isnum(exp_t *ep)
{
  return isint(symp(ep), strlen(symp(ep)), NULL);
}

/* Test if the expression is constant. */
static inline int
iscst(exp_t *ep)
{
  return iseq(ep, false) || iseq(ep, true) || isnull(ep);
}

/* Test if the expression is self-evaluating */
int
isself(exp_t *ep)
{
  return isnum(ep) || isstr(ep) || iscst(ep);
}
