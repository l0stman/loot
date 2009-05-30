#ifndef EXP_H
#define EXP_H

enum type { ATOM, PAIR, PROC };

#define symp(ep)	(ep)->u.sp
#define pairp(ep)	(ep)->u.cp
#define procp(ep)	(ep)->u.pp

struct exp {
  enum type tp;	/* type of the expression */
  union {
	char *sp;			/* pointer to the symbol of an atom */
	struct cons *cp;	/* pointer to a pair */
	struct proc *pp;	/* pointer to a procedure */
  } u;
};

#define car(cp)	(cp)->car
#define cdr(cp)	(cp)->cdr

struct cons {	/* pair */
  struct exp *car;
  struct exp *cdr;
};

#define fpar(fp)	(fp)->parp
#define fbody(fp)	(fp)->bodyp
#define fenv(fp)	(fp)->envp

struct func {	/* Represents a function */
  struct exp *parp;	/* Parameters of the function */
  struct exp *bodyp;	/* body of the function */
  struct env *envp;		/* environment of the function */
};

enum ftype { FUNC, PRIM };
struct proc {	/* A procedure is a function or a primitive */
  enum ftype tp;	/* type of the procedure */
  char *label;		/* label of the procedure */
  union {
	struct exp *(*primp)();	/* pointer to a primitive function */
	struct func *funcp;	/* pointer to an user-defined function */
  } u;
};

extern const struct exp false;
extern const struct exp true;
extern struct exp null;

int iseq(const struct exp *, const struct exp *);
int islist(const struct exp *);
char *tostr(const struct exp *);

static __inline__ int
isatom(const struct exp *ep)
{
  return (ep != NULL && ep->tp == ATOM);
}

static __inline__ int
ispair(const struct exp *ep)
{
  return (ep != NULL && ep->tp == PAIR);
}

static __inline__ int
isproc(const struct exp *ep)
{
  return (ep != NULL && ep->tp == PROC);
}

/* Return an atom whose symbol is the parameter */
static __inline__ struct exp *
atom(char *s)
{
  struct exp *ep;

  ep = smalloc(sizeof(*ep));
  ep->tp = ATOM;
  ep->u.sp = sstrdup(s);
  return ep;
}

/* Return a pair of expression */
static __inline__ struct exp *
cons(struct exp *a, struct exp *b)
{
  struct exp *ep;

  ep = smalloc(sizeof(*ep));
  ep->tp = PAIR;
  ep->u.cp = smalloc(sizeof(*ep->u.cp));
  ep->u.cp->car = a;
  ep->u.cp->cdr = b;
  return ep;
}

/* Test if the expression is null */
static __inline__ int
isnull(const struct exp *ep)
{
  return iseq(ep, &null);
}
#endif /* !EXP_H */
