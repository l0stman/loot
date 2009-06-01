#ifndef EXP_H
#define EXP_H

enum type { ATOM, PAIR, PROC };

#define type(ep)	(ep)->tp
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

#define car(ep)	pairp(ep)->car
#define cdr(ep)	pairp(ep)->cdr

struct cons {	/* pair */
  struct exp *car;
  struct exp *cdr;
};

struct func {	/* Represents a function */
  struct exp *parp;	/* Parameters of the function */
  struct exp *bodyp;	/* body of the function */
  struct env *envp;		/* environment of the function */
};

#define primp(ep)	procp(ep)->u.primp
#define funcp(ep)	procp(ep)->u.funcp
#define fpar(ep)	funcp(ep)->parp
#define fbody(ep)	funcp(ep)->bodyp
#define fenv(ep)	funcp(ep)->envp

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
  return (ep != NULL && type(ep) == ATOM);
}

static __inline__ int
ispair(const struct exp *ep)
{
  return (ep != NULL && type(ep) == PAIR);
}

static __inline__ int
isproc(const struct exp *ep)
{
  return (ep != NULL && type(ep) == PROC);
}

/* Return an atom whose symbol is s */
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

/* Return a function */
static __inline__ struct proc *
func(struct exp *parp, struct exp *bodyp, struct env *envp)
{
  struct func *fp;
  struct proc *pp;

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

/* Return an expression from a procedure */
static __inline__ struct exp *
proc(struct proc *pp)
{
  struct exp *ep;

  ep = smalloc(sizeof(*ep));
  ep->tp = PROC;
  ep->u.pp = pp;
  return ep;
}

/* Test if the expression is null */
static __inline__ int
isnull(const struct exp *ep)
{
  return iseq(ep, &null);
}

/* Print the message and return a null pointer */
static __inline__ struct exp *
everr(char *msg, struct exp *ep)
{
  char *s;
  
  warnx("%s: %s", msg, s = tostr(ep));
  free(s);
  return NULL;
}
#endif /* !EXP_H */
