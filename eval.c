#include "loot.h"
#include "exp.h"
#include "env.h"
#include "eval.h"
#include "type.h"

static struct exp *evdef(struct exp *, struct env *);
static struct exp *evvar(struct exp *, struct env *);
static struct exp *evquote(struct exp *);
static struct exp *evif(struct exp *, struct env *);
static struct exp *evbegin(struct exp *, struct env *);
static struct exp *evcond(struct exp *, struct env *);
static struct exp *evlambda(struct exp *, struct env *);
static struct exp *evapply(struct exp *, struct env *);

/* Evaluate the expression */
struct exp *
eval(struct exp *ep, struct env *envp)
{
  if (isself(ep))
	return ep;
  else if (isdef(ep))
	return evdef(ep, envp);
  else if (isvar(ep))
	return evvar(ep, envp);
  else if (isquote(ep))
	return evquote(ep);
  else if (isif(ep))
	return evif(ep, envp);
  else if (isbegin(ep))
	return evbegin(ep, envp);
  else if (iscond(ep))
	return evcond(ep, envp);
  else if (islambda(ep))
	return evlambda(ep, envp);
  else if (islist(ep))	/* application */
	return evapply(ep, envp);
  else
	return everr("unknown expression", ep);
}

/* Evaluate a define expression */
static int bind(char **, struct exp **, struct exp *);
static int chknum(struct exp *, int);

static struct exp *
evdef(struct exp *ep, struct env *envp)
{
  char *var = NULL;
  struct exp *val = NULL;
   
  if (!chknum(ep, 3))
	return NULL;
  if (bind(&var, &val, cdr(ep)) && (val = eval(val, envp)) != NULL) {
	if (type(val) == PROC && procp(val)->label == NULL)
	  val->u.pp->label = sstrdup(var);	/* label anonymous procedure */
	install(var, val, envp);
  }
  return NULL;
}

/* Bind a variable with a value */
static int
bind(char **varp, struct exp **valp, struct exp *lp)
{
  struct exp *ep;
  
  if (ispair(ep = car(lp))) {	/* lambda shortcut */
	if (!issym(car(ep))) {
	  everr("should be a symbol", car(ep));
	  return 0;
	}
	*varp = symp(car(ep));
	*valp = cons(atom("lambda"), cons(cdr(ep), cdr(lp)));
	return 1;
  }
  if (issym(ep)) {
	*varp = symp(ep);
	*valp = car(cdr(lp));
	return 1;
  }
  everr("the expression couldn't be defined", car(lp));
  return 0;
}

/* Return true if the expression length is equal to n */
static int
chknum(struct exp *lp, int n)
{
  struct exp *ep;
  
  for(ep = lp; n && !isnull(ep); n--, ep = cdr(ep))
	;
  if (n == 0 && isnull(ep))
	return 1;
  everr("wrong number of expressions", lp);
  return 0;
}

/* Return the value of a variable if any */
static struct exp *
evvar(struct exp *ep, struct env *envp)
{
  struct nlist *np;

  if ((np = lookup(symp(ep), envp)) == NULL)
	return everr("unbound variable", ep);
  return np->defn;
}

/* Return the quoted expression */
static struct exp *
evquote(struct exp *ep)
{
  return (chknum(ep, 2) ? car(cdr(ep)): NULL);
}

/* Evaluate an if expression */
static struct exp *
evif(struct exp *ep, struct env *envp)
{
  struct exp *b, *res;
  
  if (!chknum(ep, 4))
	return NULL;
  ep = cdr(ep);
  if ((b = eval(car(ep), envp)) == NULL)
	return NULL;
  res = (iseq(&false, b) ? car(cdr(cdr(ep))): car(cdr(ep)));
  return eval(res, envp);
}

/* Evaluate a begin expression */
static struct exp *
evbegin(struct exp *ep, struct env *envp)
{
  struct exp *form = NULL;

  for (ep = cdr(ep); !isnull(ep); ep = cdr(ep))
	form = eval(car(ep), envp);
  return form;
}

/* Evaluate a cond expression */
static struct exp *
evcond(struct exp *ep, struct env *envp)
{
  struct exp *clause;
  struct exp _else_ = { ATOM, {"else"} };

  for (ep = cdr(ep); !isnull(ep); ep = cdr(ep)) {
	if (!islist(car(ep)))
	  return everr("should be a list", car(ep));
	if (iseq(&_else_, car(clause = car(ep))) ||
		!iseq(&false, eval(car(clause), envp)))
	  return eval(cons(atom("begin"), cdr(clause)), envp);
  }
  return NULL;
}

/* Evaluate a lambda expression */
static int chkpars(struct exp *);

static struct exp *
evlambda(struct exp *lp, struct env *envp)
{
  struct exp *ep;
   
  ep = cdr(lp);
  if (isnull(ep) || !chkpars(car(ep)) || isnull(cdr(ep)))
	return everr("syntax error in", lp);
  return proc(func(car(ep), cons(atom("begin"), cdr(ep)), envp));
}

/* Verify if the expression represents function's parameters */
static int
chkpars(struct exp *ep)
{
  if (!ispair(ep))
	return isatom(ep);
  for (; !isatom(ep); ep = cdr(ep))
	if (!issym(car(ep))) {
	  everr("should be a symbol", car(ep));
	  return 0;
	}
  return 1;
}

/* Eval a compound expression */
static struct exp *evmap(struct exp *, struct env *);

static struct exp *
evapply(struct exp *ep, struct env *envp)
{
  struct exp *op;
  struct exp *args;
  struct exp *parp;
  struct exp *blist;	/* binding list */

  if (!isproc(op = eval(car(ep), envp)))
	return everr("expression is not a procedure", car(ep));
  if ((args = evmap(cdr(ep), envp)) == NULL)
	return NULL;
  if (procp(op)->tp == PRIM)	/* primitive */
	return primp(op)(args, envp);
  
  /* function */
  for (parp = fpar(op), blist = &null ; !isatom(parp);
	   parp = cdr(parp), args = cdr(args)) {
	if (isnull(args))
	  return everr("too few arguments provided to", car(ep));
	blist = cons(cons(car(parp), car(args)), blist);
  }
  if (isnull(parp)) {
	if (!isnull(args))
	  return everr("too many arguments provided to", car(ep));
  }
  else	/* variable length arguments */
	blist = cons(cons(parp, args), blist);
  return eval(fbody(op), extenv(blist, fenv(op)));
}

/* Return a list of evaluated expressions */
static struct exp *
evmap(struct exp *lp, struct env *envp)
{
  struct exp *ep, *res, *memp;

  for (memp = &null; !isnull(lp); lp = cdr(lp)) {
	if ((ep = eval(car(lp), envp)) == NULL)	/* an error occured */
	  return NULL;
	memp = cons(ep, memp);
  }
  /* reverse the list */
  for (res = &null; !isnull(memp); memp = cdr(memp))
	res = cons(car(memp), res);
  return res;
}
