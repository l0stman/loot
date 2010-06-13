#ifndef TYPE_H
#define TYPE_H

extern int isintstr(const char *, int);
extern int isfloatstr(const char *, int);
extern int isratstr(const char *, int);
extern int isself(exp_t *);

/* Index of tag symbols in tagsyms. */
enum tag {
        DEFINE,
        QUOTE,
        IF,
        BEGIN,
        COND,
        LAMBDA,
        AND,
        OR,
        LET,
        SET,
        SETCAR,
        SETCDR
};

extern const char *tagsyms[];
extern void inittags(void);

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
istag(exp_t *ep, char const *tag)
{
        return ispair(ep) && isatom(car(ep)) && tag == symp(car(ep));
}

/* Test if the expression is a definition. */
static inline int
isdef(exp_t *ep)
{
        return istag(ep, tagsyms[DEFINE]);
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
        return istag(ep, tagsyms[QUOTE]);
}

/* Test if the expression is an if expression */
static inline int
isif(exp_t *ep)
{
        return istag(ep, tagsyms[IF]);
}

/* Test if the expression is a cond expression */
static inline int
iscond(exp_t *ep)
{
        return istag(ep, tagsyms[COND]);
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
        return istag(ep, tagsyms[BEGIN]);
}

/* Test if an expression is an and expression */
static inline int
isand(exp_t *ep)
{
        return istag(ep, tagsyms[AND]);
}

/* Test if an expression is an or expression */
static inline int
isor(exp_t *ep)
{
        return istag(ep, tagsyms[OR]);
}

/* Test if an expression is a lambda expression */
static inline int
islambda(exp_t *ep)
{
        return istag(ep, tagsyms[LAMBDA]);
}

/* Test if an expression is a let expression */
static inline int
islet(exp_t *ep)
{
        return istag(ep, tagsyms[LET]);
}

/* Test if an expression is a set! expression */
static inline int
isset(exp_t *ep)
{
        return istag(ep, tagsyms[SET]);
}

/* Test if an expression is a set-car! expression */
static inline int
issetcar(exp_t *ep)
{
        return istag(ep, tagsyms[SETCAR]);
}

/* Test if an expression is a set-cdr! expression */
static inline int
issetcdr(exp_t *ep)
{
        return istag(ep, tagsyms[SETCDR]);
}
#endif /* !TYPE_H */
