#ifndef TYPE_H
#define TYPE_H

extern int isint(const char *, int);
extern int isfloat(const char *, int);
extern int isnum(exp_t *);
extern int isself(exp_t *);

/* Test if exp is a list whose first element is an atom of symbol tag. */
static inline int
istag(exp_t *ep, char *tag)
{
  return islist(ep) && isatom(car(ep)) &&
	strcmp(tag, symp(car(ep))) == 0;
}

/* Test if the expression is a definition. */
static inline int
isdef(exp_t *ep)
{
  return istag(ep, "define");
}

/* Test if the expression is a symbol. */
static inline int
issym(exp_t *ep)
{
  return isatom(ep) && symp(ep) != NULL && !isself(ep);
}

/* Test if the expression is a string. */
static inline int
isstr(exp_t *ep)
{
  return isatom(ep) && *symp(ep) == '"';
}

/* Test if the expression is a variable. */
static inline int
isvar(exp_t *ep)
{
  return issym(ep);
}

/* Test if the expression is a quote */
static inline int
isquote(exp_t *ep)
{
  return istag(ep, "quote");
}

/* Test if the expression is an if expression */
static inline int
isif(exp_t *ep)
{
  return istag(ep, "if");
}

/* Test if the expression is a cond expression */
static inline int
iscond(exp_t *ep)
{
  return istag(ep, "cond");
}

/* Test if an expression is a boolean */
static inline int
isbool(exp_t *ep)
{
  return ep != NULL && (iseq(true, ep) || iseq(false, ep));
}

/* Test if an expression is a begin expression */
static inline int
isbegin(exp_t *ep)
{
  return istag(ep, "begin");
}

/* Test if an expression is an and expression */
static inline int
isand(exp_t *ep)
{
  return istag(ep, "and");
}

/* Test if an expression is an or expression */
static inline int
isor(exp_t *ep)
{
  return istag(ep, "or");
}

/* Test if an expression is a lambda expression */
static inline int
islambda(exp_t *ep)
{
  return istag(ep, "lambda");
}

/* Test if an expression is a let expression */
static inline int
islet(exp_t *ep)
{
  return istag(ep, "let");
}
#endif /* !TYPE_H */
