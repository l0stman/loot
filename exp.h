#ifndef EXP_H
#define EXP_H

#include "atom.h"

enum type { ATOM, PAIR, PROC };

#define type(ep)	(ep)->tp
#define symp(ep)	(ep)->u.sp
#define pairp(ep)	(ep)->u.cp
#define procp(ep)	(ep)->u.pp
#define label(ep)	(ep)->u.pp->label

typedef struct exp {
  enum type tp;	/* type of the expression */
  union {
	const char *sp;		/* pointer to the symbol of an atom */
	struct cons *cp;	/* pointer to a pair */
	struct proc *pp;	/* pointer to a procedure */
  } u;
} exp_t;

#define car(ep)	pairp(ep)->car
#define cdr(ep)	pairp(ep)->cdr

struct cons {	/* pair */
  exp_t *car;
  exp_t *cdr;
};

struct func {	/* Represents a function */
  exp_t *parp;	/* Parameters of the function */
  exp_t *bodyp;	/* body of the function */
  struct env *envp;		/* environment of the function */
};

#define primp(ep)	procp(ep)->u.primp
#define funcp(ep)	procp(ep)->u.funcp
#define fpar(ep)	funcp(ep)->parp
#define fbody(ep)	funcp(ep)->bodyp
#define fenv(ep)	funcp(ep)->envp

enum ftype { FUNC, PRIM };
typedef struct proc {	/* A procedure is a function or a primitive */
  enum ftype tp;	/* type of the procedure */
  char *label;		/* label of the procedure */
  union {
	exp_t *(*primp)();	/* pointer to a primitive function */
	struct func *funcp;	/* pointer to an user-defined function */
  } u;
} proc_t;

extern const exp_t false;
extern const exp_t true;
extern exp_t null;

int iseq(const exp_t *, const exp_t *);
int islist(const exp_t *);
char *tostr(const exp_t *);

static inline int
isatom(const exp_t *ep)
{
  return (ep != NULL && type(ep) == ATOM);
}

static inline int
ispair(const exp_t *ep)
{
  return (ep != NULL && type(ep) == PAIR);
}

static inline int
isproc(const exp_t *ep)
{
  return (ep != NULL && type(ep) == PROC);
}

/* Return an atom whose symbol is s */
static inline exp_t *
atom(const char *s)
{
  exp_t *ep;

  ep = smalloc(sizeof(*ep));
  ep->tp = ATOM;
  ep->u.sp = strtoatm(s);
  return ep;
}

/* Return a pair of expression */
static inline exp_t *
cons(exp_t *a, exp_t *b)
{
  exp_t *ep;

  ep = smalloc(sizeof(*ep));
  ep->tp = PAIR;
  ep->u.cp = smalloc(sizeof(*ep->u.cp));
  ep->u.cp->car = a;
  ep->u.cp->cdr = b;
  return ep;
}

/* Return a function */
static inline proc_t *
func(exp_t *parp, exp_t *bodyp, struct env *envp)
{
  struct func *fp;
  proc_t *pp;

  fp = smalloc(sizeof(*fp));
  fp->parp = parp;
  fp->bodyp = bodyp;
  fp->envp = envp;
  
  pp = smalloc(sizeof(*pp));
  pp->tp = FUNC;
  pp->label = NULL;	/* anonymous procedure */
  pp->u.funcp = fp;
  return pp;
}

/* Return a primitive */
static inline proc_t *
prim(char *label, exp_t *(primp)())
{
  proc_t *pp;

  pp = smalloc(sizeof(*pp));
  pp->tp = PRIM;
  pp->label = label;
  pp->u.primp = primp;
  return pp;
}

/* Return an expression from a procedure */
static inline exp_t *
proc(proc_t *pp)
{
  exp_t *ep;

  ep = smalloc(sizeof(*ep));
  ep->tp = PROC;
  ep->u.pp = pp;
  return ep;
}

/* Test if the expression is null */
static inline int
isnull(const exp_t *ep)
{
  return iseq(ep, &null);
}
#endif /* !EXP_H */
