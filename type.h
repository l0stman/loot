#ifndef TYPE_H
#define TYPE_H

extern int isintstr(const char *, int);
extern int isfloatstr(const char *, int);
extern int isratstr(const char *, int);
extern int isself(exp_t *);

/* Test if an expression is an integer */
static inline int
isint(exp_t *ep)
{
        return isfxn(ep);
}

/* Test if the expression is a number */
static inline int
isnum(exp_t *ep)
{
        return (isint(ep) || israt(ep) || isfloat(ep));
}

/* Test if exp is a list whose first element is an atom of symbol tag. */
static inline int
istag(exp_t *ep, exp_t *tag)
{
        return ispair(ep) && iseq(car(ep), tag);
}

/* Test if the expression is a definition. */
static inline int
isdef(exp_t *ep)
{
        return istag(ep, keywords[DEFINE]);
}

/* Test if the expression is a symbol. */
static inline int
issym(exp_t *ep)
{
        return isatom(ep) && !isself(ep) && !isnull(ep);
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
        return istag(ep, keywords[QUOTE]);
}

/* Test if the expression is an if expression */
static inline int
isif(exp_t *ep)
{
        return istag(ep, keywords[IF]);
}

/* Test if the expression is a cond expression */
static inline int
iscond(exp_t *ep)
{
        return istag(ep, keywords[COND]);
}

/* Test if an expression is a begin expression */
static inline int
isbegin(exp_t *ep)
{
        return istag(ep, keywords[BEGIN]);
}

/* Test if an expression is an and expression */
static inline int
isand(exp_t *ep)
{
        return istag(ep, keywords[AND]);
}

/* Test if an expression is an or expression */
static inline int
isor(exp_t *ep)
{
        return istag(ep, keywords[OR]);
}

/* Test if an expression is a lambda expression */
static inline int
islambda(exp_t *ep)
{
        return istag(ep, keywords[LAMBDA]);
}

/* Test if an expression is a let expression */
static inline int
islet(exp_t *ep)
{
        return istag(ep, keywords[LET]);
}

/* Test if an expression is a set! expression */
static inline int
isset(exp_t *ep)
{
        return istag(ep, keywords[SET]);
}

/* Test if an expression is a set-car! expression */
static inline int
issetcar(exp_t *ep)
{
        return istag(ep, keywords[SETCAR]);
}

/* Test if an expression is a set-cdr! expression */
static inline int
issetcdr(exp_t *ep)
{
        return istag(ep, keywords[SETCDR]);
}

/* Test if an expression is a quasi-quote expression */
static inline int
isqquote(exp_t *ep)
{
        return istag(ep, keywords[QQUOTE]);
}

/* Test if an expression is an unquote expression */
static inline int
isunquote(exp_t *ep)
{
        return istag(ep, keywords[UNQUOTE]);
}

/* Test if an expression is an unquote-splicing expression */
static inline int
issplice(exp_t *ep)
{
        return istag(ep, keywords[SPLICE]);
}
#endif /* !TYPE_H */
