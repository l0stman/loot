#include "loot.h"
#include "exp.h"
#include "type.h"

/* Returns the length of suffix of s representing an integer. */
static int
intlen(const char *s, int len)
{
  const char *cp;
  
  if (*(cp = s) == '+' || *cp == '-') {
	if (len == 1)
	  return 0;
	cp++;
  }
  for (; cp-s < len && isdigit(*cp); cp++)
	;
  return cp-s;
}

/* Test if the string is an integer */
int
isintstr(const char *s, int len)
{
  return intlen(s, len) == len;
}

/* Test if the string is a float */
int
isfloatstr(const char *s, int len)
{
  int offset;

  if ((offset = intlen(s, len)) == 0)
	return 0;
  s += offset, len -= offset;
  switch (*s) {
  case '.':
	if (!isdigit(*(s+1)) ||
		(offset = intlen(++s, --len)) == 0 ||
		(*(s+offset) != 'e' && *(s+offset) != 'E'))
	  return (offset && *(s+offset) == '\0' ? 1 : 0);
	s += offset, len -= offset;
  case 'e':
  case 'E':
	return (--len && (intlen(++s, len) == len) ? 1 : 0);
  break;
  }
  return 0;
}

/* Test if the expression is a number */
int
isnum(exp_t *ep)
{
  return isfloatstr(symp(ep), strlen(symp(ep)));
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
