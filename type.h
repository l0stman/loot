#ifndef TYPE_H
#define TYPE_H

int isnum(exp_t *);
int isself(exp_t *);

/* Test if exp is a list whose first element is an atom of symbol tag. */
static __inline__ int
istag(exp_t *ep, char *tag)
{
  return islist(ep) && isatom(car(ep)) &&
	strcmp(tag, symp(car(ep))) == 0;
}

/* Test if the expression is a definition. */
static __inline__ int
isdef(exp_t *ep)
{
  return istag(ep, "define");
}

/* Test if the expression is a symbol. */
static __inline__ int
issym(exp_t *ep)
{
  return isatom(ep) && symp(ep) != NULL && !isself(ep);
}

/* Test if the expression is a string. */
static __inline__ int
isstr(exp_t *ep)
{
  return isatom(ep) && *symp(ep) == '"';
}

/* Test if the expression is a variable. */
static __inline__ int
isvar(exp_t *ep)
{
  return issym(ep);
}

/* Test if the expression is a quote */
static __inline__ int
isquote(exp_t *ep)
{
  return istag(ep, "quote");
}

/* Test if the expression is an if expression */
static __inline__ int
isif(exp_t *ep)
{
  return istag(ep, "if");
}

/* Test if the expression is a cond expression */
static __inline__ int
iscond(exp_t *ep)
{
  return istag(ep, "cond");
}

/* Test if an expression is a boolean */
static __inline__ int
isbool(exp_t *ep)
{
  return ep != NULL && (iseq(&true, ep) || iseq(&false, ep));
}

/* Test if an expression is a begin expression */
static __inline__ int
isbegin(exp_t *ep)
{
  return istag(ep, "begin");
}

/* Test if an expression is an and expression */
static __inline__ int
isand(exp_t *ep)
{
  return istag(ep, "and");
}

/* Test if an expression is an or expression */
static __inline__ int
isor(exp_t *ep)
{
  return istag(ep, "or");
}

/* Test if an expression is a lambda expression */
static __inline__ int
islambda(exp_t *ep)
{
  return istag(ep, "lambda");
}

#endif /* !TYPE_H */
