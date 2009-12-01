#include "loot.h"
#include "exp.h"
#include "atom.h"
#include "type.h"
#include "parser.h"

static exp_t *parse_atm(char *, int);
static exp_t *parse_pair(char *, int);

/* Parse a non-empty expression */
exp_t *
parse(char *s, int len)
{
  return (*s == '(' ? parse_pair(s+1, len-1): parse_atm(s, len));
}

/* Parse a non-pair expression */
static exp_t *
parse_atm(char *s, int len)
{
  exp_t *ep;
  char *p;
  char *endp[1];
  long n, d;

  p = sstrndup(s, len);
  if (isfloatstr(p, len))
	ep = nfloat(atof(p));
  else if (isratstr(p, len)) {
	n = strtol(p, endp, 10);
	d = strtol(++*endp, NULL, 10);
	ep = nrat(n, d);
  } else
	ep = atom(p);
  free(p);
  return ep;
}

/* Parse a pair of expressions */
static int carlen(char *);

static exp_t *
parse_pair(char *s, int size)
{
  exp_t *car, *cdr;
  int len = size;
  char *cp = s;
  int n;
  
  if (*cp == ')')
	return null;
  n = carlen(cp);
  if (*cp == '.' && n == 1)
	goto fail;
  car = parse(cp, n);
  if (*(cp+n) == ' ')
	n++;
  cp += n, len -= n;
  if (*cp == '.' && *(cp+1) == ' ') {	/* dotted-pair notation */
	cp += 2, len -= 3;
	if (*cp == ')' || *(cp+carlen(cp)) != ')')
	  goto fail;
	cdr = parse(cp, len);
  } else {
	if (*cp == '.' && *(cp+1) == ')')
	  goto fail;
	cdr = parse_pair(cp, len);
  }
  return cons(car, cdr);
 fail:
  warnx("Illegal use of . -- (%.*s", size, s);
  return NULL;
}

/*
 * Return the length of the first expression in a string
 * representing a non-null pair.
 */
static int
carlen(char *s)
{
  int pn;	/* number of open parenthesis */ 
  int len = 1;
  char c;

  if (*s != '(') {	/* the first expression is an atom */
	for (c = *s++; !isstop(c, *s++); len++)	
	  ;
	if (c == '"')
	  len++;	/* count also the ending quote */
  } else
	for (pn = 1; pn; len++)
	  switch (*++s) {
	  case '(':
		++pn;
		break;
	  case ')':
		--pn;
		break;
	  default:
		break;
	  }
  return len;
}
