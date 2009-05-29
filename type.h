#ifndef TYPE_H
#define TYPE_H

int isnum(struct exp *);
int isself(struct exp *);

/* Test if exp is pair whose first element is an atom of symbol tag. */
static __inline__ int
istag(struct exp *ep, char *tag)
{
  return (!ispair(ep) || ispair(car(pairp(ep))) ? 0:
		  strcmp(tag, symp(car(pairp(ep)))) == 0);
}

/* Test if the expression is a definition. */
static __inline__ int
isdef(struct exp *ep)
{
  return istag(ep, "define");
}

/* Test if the expression is a symbol. */
static __inline__ int
issym(struct exp *ep)
{
  return isatom(ep) && symp(ep) != NULL && !isself(ep);
}

/* Test if the expression is a variable. */
static __inline__ int
isvar(struct exp *ep)
{
  return issym(ep);
}

/* Test if the expression is a quote */
static __inline__ int
isquote(struct exp *ep)
{
  return istag(ep, "quote");
}

/* Test if the expression is an if expression */
static __inline__ int
isif(struct exp *ep)
{
  return istag(ep, "if");
}

/* Test if the expression is a cond expression */
static __inline__ int
iscond(struct exp *ep)
{
  return istag(ep, "cond");
}

/* Test if an expression is a boolean */
static __inline__ int
isbool(struct exp *ep)
{
  return ep != NULL && (iseq(&true, ep) || iseq(&false, ep));
}

/* Test if an expression is a begin expression */
static __inline__ int
isbegin(struct exp *ep)
{
  return istag(ep, "begin");
}

/* Test if an expression is a lambda expression */
static __inline__ int
islambda(struct exp *ep)
{
  return istag(ep, "lambda");
}

#endif /* !TYPE_H */
