#include "loot.h"
#include "exp.h"
#include "atom.h"
#include "parser.h"

static exp_t *parse_atm(char *, int);
static exp_t *parse_pair(char *, int);

/* Parse a non-empty expression */
exp_t *
parse(char *s, int len)
{
  return (*s == '(' ? parse_pair(s+1, len-1): parse_atm(s, len));
}

/* Parse an atom expression */
static exp_t *
parse_atm(char *s, int len)
{
  exp_t *ep;

  ep = smalloc(sizeof(*ep));
  ep->tp = ATOM;
  ep->u.sp = natom(s, len);
  return ep;
}

/* Parse a pair of expressions */
static int carlen(char *);

static exp_t *
parse_pair(char *s, int len)
{
  exp_t *car, *cdr;
  int n;
  
  if (*s == ')')
	return &null;
  n = carlen(s);
  car = parse(s, n);
  if (*(s+n) == ' ')
	n++;
  s += n, len -= n;
  if (*s == '.' && *(s+1+carlen(s+1)) != ')')
	err_quit("Illegal use of .");
  cdr = (*s == '.' ? parse(s+1, len-2): parse_pair(s, len));
  return cons(car, cdr);
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
