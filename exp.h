#ifndef EXP_H
#define EXP_H

#include "atom.h"

#define TYPES                                   \
                X(atom, ATOM) SEP               \
                X(pair, PAIR) SEP               \
                X(proc, PROC) SEP               \
                X(float, FLOAT) SEP             \
                X(rat, RAT) SEP                 \
                X(fxn, FIXNUM) SEP              \
                X(char, CHAR) SEP               \
                X(bool, BOOL) SEP               \
                X(str, STRING)

#define X(a, b) b
#define SEP     ,
enum type { TYPES };
#undef SEP
#undef X

#define type(ep)        (ep)->tp
#define symp(ep)        (ep)->u.sp
#define pairp(ep)       (ep)->u.cp
#define procp(ep)       (ep)->u.pp
#define label(ep)       (ep)->u.pp->label
#define flt(ep)         (ep)->u.ft
#define fixnum(ep)      (ep)->u.fx
#define ratp(ep)        (ep)->u.rp
#define char(ep)        (ep)->u.ch
#define strp(ep)        (ep)->u.str

typedef struct exp {
        enum type            tp; /* type of the expression */
        union {
                symb_t      *sp; /* pointer to the symbol of an atom */
                struct cons *cp; /* pointer to a pair */
                struct proc *pp; /* pointer to a procedure */
                int          fx; /* represents a fixnum */
                struct rat  *rp; /* pointer to a rational */
                double       ft; /* represents a float */
                char         ch; /* represents a character */
                struct str  *str; /* pointer to a string */
        } u;
} exp_t;

#define car(ep)   pairp(ep)->car
#define cdr(ep)   pairp(ep)->cdr
#define caar(ep)  car(car(ep))
#define cadr(ep)  car(cdr(ep))
#define cdar(ep)  cdr(car(ep))
#define cddr(ep)  cdr(cdr(ep))
#define caddr(ep) car(cddr(ep))
#define cadar(ep) car(cdar(ep))
#define cdddr(ep) cdr(cddr(ep))
#define cddar(ep) cdr(cdar(ep))

struct cons {   /* pair */
        exp_t *car;
        exp_t *cdr;
};


/* Represents an evaluation procedure (see analyze). */
typedef struct evproc {
        exp_t *(*eval)();
        void *argv;
} evproc_t;

struct func {                   /* Represents a function */
        exp_t      *parp;       /* Parameters of the function */
        evproc_t   *bodyp;      /* body of the function */
        struct env *envp;       /* environment of the function */
};

#define ptype(ep)       procp(ep)->tp
#define primp(ep)       procp(ep)->u.primp
#define funcp(ep)       procp(ep)->u.funcp
#define fpar(ep)        funcp(ep)->parp
#define fbody(ep)       funcp(ep)->bodyp
#define fenv(ep)        funcp(ep)->envp

enum ftype { FUNC, PRIM };
typedef struct proc {           /* A procedure is a function or a primitive */
        enum ftype tp;          /* type of the procedure */
        symb_t *label;          /* label of the procedure */
        union {
                exp_t *(*primp)();  /* pointer to a primitive function */
                struct func *funcp; /* pointer to an user-defined function */
        } u;
} proc_t;

#define num(ep) ratp(ep)->num
#define den(ep) ratp(ep)->den

typedef struct rat {            /* represents a rational */
        int num;                /* numerator */
        int den;                /* denominator */
} rat_t;

#define str(ep)  strp(ep)->s
#define slen(ep) strp(ep)->len

typedef struct str {            /* represents a string */
        char   *s;
        size_t len;
} str_t;

extern exp_t *false;
extern exp_t *true;
extern exp_t *null;
extern exp_t *undefined;
extern exp_t *unquote;
extern exp_t *splice;

#define KEYWORDS                                \
                X(DEFINE, "define"),            \
                X(QUOTE, "quote"),              \
                X(IF, "if"),                    \
                X(BEGIN, "begin"),              \
                X(COND, "cond"),                \
                X(LAMBDA, "lambda"),            \
                X(AND, "and"),                  \
                X(OR, "or"),                    \
                X(LET, "let"),                  \
                X(SET, "set!"),                 \
                X(SETCAR, "set-car!"),          \
                X(SETCDR, "set-cdr!"),          \
                X(ELSE, "else"),                \
                X(ARROW, "=>"),                 \
                X(QQUOTE, "quasiquote"),        \
                X(UNQUOTE, "unquote"),          \
                X(SPLICE, "unquote-splicing")

#define X(k, s) k
enum kindex { KEYWORDS };  /* Index of keyword symbols in keywords. */
#undef  X

extern void *keywords[];
extern void initkeys(void);

extern int iseq(const exp_t *, const exp_t *);
extern char *tostr(const exp_t *);
extern void instcst(struct env *);
extern exp_t *nrat(int, int);

#define X(NAME, TYPE)                           \
        static inline int                       \
        is##NAME(const exp_t *ep)               \
        {                                       \
                return ep && type(ep) == TYPE;  \
        }
#define SEP
TYPES
#undef X
#undef SEP

/* Return an atom whose symbol is s */
static inline exp_t *
atom(symb_t *s)
{
        exp_t *ep;

        NEW(ep);
        ep->tp = ATOM;
        ep->u.sp = strtoatm(s);
        return ep;
}

/* Return a boolean whose symbol is s. */
static inline exp_t *
bool(symb_t *s)
{
        exp_t *ep;

        NEW(ep);
        ep->tp = BOOL;
        ep->u.sp = strtoatm(s);
        return ep;
}

/* Return a pair of expression */
static inline exp_t *
cons(exp_t *a, exp_t *b)
{
        exp_t *ep;

        NEW(ep);
        ep->tp = PAIR;
        NEW(ep->u.cp);
        ep->u.cp->car = a;
        ep->u.cp->cdr = b;
        return ep;
}

static inline evproc_t *
nevproc(exp_t *(*eval)(), void *argv)
{
        evproc_t *epp;

        NEW(epp);
        epp->eval = eval;
        epp->argv = argv;
        return epp;
}

/* Return a function */
static inline proc_t *
nfunc(exp_t *parp, evproc_t *bodyp, struct env *envp)
{
        struct func *fp;
        proc_t *pp;

        NEW(fp);
        fp->parp = parp;
        fp->bodyp = bodyp;
        fp->envp = envp;

        NEW(pp);
        pp->tp = FUNC;
        pp->label = NULL;     /* anonymous procedure */
        pp->u.funcp = fp;
        return pp;
}

/* Return a primitive */
static inline proc_t *
nprim(char *label, exp_t *(primp)())
{
        proc_t *pp;

        NEW(pp);
        pp->tp = PRIM;
        pp->label = label;
        pp->u.primp = primp;
        return pp;
}

/* Return an expression from a procedure */
static inline exp_t *
nproc(proc_t *pp)
{
        exp_t *ep;

        NEW(ep);
        ep->tp = PROC;
        ep->u.pp = pp;
        return ep;
}

/* Return an expression representing a float */
static inline exp_t *
nfloat(double e)
{
        exp_t *ep;

        NEW(ep);
        type(ep) = FLOAT;
        flt(ep) = e;
        return ep;
}

/* Return an expression representing a fixnum */
static inline exp_t *
nfixnum(int i)
{
        exp_t *ep;

        NEW(ep);
        type(ep) = FIXNUM;
        fixnum(ep) = i;
        return ep;
}

/* Return an expression representing a character */
static inline exp_t *
nchar(char c)
{
        exp_t *ep;

        NEW(ep);
        type(ep) = CHAR;
        char(ep) = c;
        return ep;
}

/* Return an expression representing a string. */
static inline exp_t *
nstr(const char *s, size_t len)
{
        exp_t *ep;

        NEW(ep);
        type(ep) = STRING;
        NEW(strp(ep));
        str(ep) = sstrndup(s, len);
        slen(ep) = len;
        return ep;
}

/* Test if the expression is null */
static inline int
isnull(const exp_t *ep)
{
        return ep != NULL && iseq(ep, null);
}

/* Return true if the expression is a null-terminated pair */
static inline int
islist(const exp_t *ep)
{
        while (ispair(ep))
                ep = cdr(ep);
        return isnull(ep);
}

/* Reverse a list */
static inline exp_t *
reverse(const exp_t *lp)
{
        exp_t *res;

        for (res = null; !isnull(lp); lp = cdr(lp))
                res = cons(car(lp), res);
        return res;
}

/* Reverse a list destructively */
static inline exp_t *
nreverse(exp_t *lp)
{
        exp_t *tail, *rest;

        for (tail = null; !isnull(lp); lp = rest) {
                rest = cdr(lp);
                cdr(lp) = tail;
                tail = lp;
        }
        return tail;
}

/* Concatenate two lists. */
static inline exp_t *
nconc(exp_t *lp1, exp_t *lp2)
{
        exp_t *p;

        if (isnull(lp1))
                return lp2;
        for (p = lp1; !isnull(cdr(p)); p = cdr(p))
                ;
        cdr(p) = lp2;
        return lp1;
}
#endif /* !EXP_H */
