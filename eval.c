#include "loot.h"
#include "exp.h"
#include "env.h"
#include "eval.h"
#include "type.h"

static exp_t *evdef(exp_t *, env_t *);
static exp_t *evvar(exp_t *, env_t *);
static exp_t *evquote(exp_t *);
static exp_t *evif(exp_t *, env_t *);
static exp_t *evbegin(exp_t *, env_t *);
static exp_t *evcond(exp_t *, env_t *);
static exp_t *evlambda(exp_t *, env_t *);
static exp_t *evand(exp_t *, env_t *);
static exp_t *evor(exp_t *, env_t *);
static exp_t *evlet(exp_t *, env_t *);
static exp_t *evapply(exp_t *, env_t *);

/* Evaluate the expression */
exp_t *
eval(exp_t *ep, env_t *envp)
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
  else if (isand(ep))
	return evand(ep, envp);
  else if (isor(ep))
	return evor(ep, envp);
  else if (islet(ep))
	return evlet(ep, envp);
  else if (islist(ep))	/* application */
	return evapply(ep, envp);
  else
	return everr("unknown expression", ep);
}

/* Evaluate a define expression */
static int bind(const char **, exp_t **, exp_t *);
static int chknum(exp_t *, int);

static exp_t *
evdef(exp_t *ep, env_t *envp)
{
  const char *var = NULL;
  exp_t *val = NULL;
   
  if (isatom(car(cdr(ep))) && !chknum(ep, 3))
	return NULL;
  if (bind(&var, &val, cdr(ep)) && (val = eval(val, envp)) != NULL) {
	if (type(val) == PROC && label(val) == NULL)
	  label(val) = strtoatm(var);	/* label anonymous procedure */
	install(var, val, envp);
  }
  return NULL;
}

/* Bind a variable with a value */
static int
bind(const char **varp, exp_t **valp, exp_t *lp)
{
  exp_t *ep;
  
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
chknum(exp_t *lp, int n)
{
  exp_t *ep;
  
  for(ep = lp; n && !isnull(ep); n--, ep = cdr(ep))
	;
  if (n == 0 && isnull(ep))
	return 1;
  everr("wrong number of expressions", lp);
  return 0;
}

/* Return the value of a variable if any */
static exp_t *
evvar(exp_t *ep, env_t *envp)
{
  struct nlist *np;

  if ((np = lookup(symp(ep), envp)) == NULL)
	return everr("unbound variable", ep);
  return np->defn;
}

/* Return the quoted expression */
static exp_t *
evquote(exp_t *ep)
{
  return (chknum(ep, 2) ? car(cdr(ep)): NULL);
}

/* Evaluate an if expression */
static exp_t *
evif(exp_t *ep, env_t *envp)
{
  exp_t *b, *res;
  
  if (!chknum(ep, 4))
	return NULL;
  ep = cdr(ep);
  if ((b = eval(car(ep), envp)) == NULL)
	return NULL;
  res = (iseq(false, b) ? car(cdr(cdr(ep))): car(cdr(ep)));
  return eval(res, envp);
}

/* Evaluate a begin expression */
static exp_t *
evbegin(exp_t *ep, env_t *envp)
{
  exp_t *form = NULL;

  for (ep = cdr(ep); !isnull(ep); ep = cdr(ep))
	form = eval(car(ep), envp);
  return form;
}

/* Evaluate a cond expression */
static exp_t *
evcond(exp_t *ep, env_t *envp)
{
  exp_t *clause, *b;
  exp_t *_else_ = atom("else");

  for (ep = cdr(ep); !isnull(ep); ep = cdr(ep)) {
	if (!islist(car(ep)))
	  return everr("should be a list", car(ep));
	clause = car(ep);
	if (iseq(_else_, b = car(clause)))
	  goto success;
	if ((b = eval(b, envp)) == NULL)	/* an error occured */
	  return NULL;
	if (!iseq(false, b))
	  goto success;
  }
  return NULL;
 success:
  return eval(cons(atom("begin"), cdr(clause)), envp);
}

/* Evaluate an and expression */
static exp_t *
evand(exp_t *ep, env_t *envp)
{
  exp_t *res = (void *)&true;
  
  for (ep = cdr(ep); !isnull(ep) && !iseq(false, res); ep = cdr(ep))
	if ((res = eval(car(ep), envp)) == NULL)
	  return NULL;
  return res;
}

/* Evaluate an or expression */
static exp_t *
evor(exp_t *ep, env_t *envp)
{
  exp_t *res = (void *)&false;

  for (ep = cdr(ep); !isnull(ep) && iseq(false, res); ep = cdr(ep))
	if ((res = eval(car(ep), envp)) == NULL)
	  return NULL;
  return res;
}

/* Evaluate a lambda expression */
static int chkpars(exp_t *);

static exp_t *
evlambda(exp_t *lp, env_t *envp)
{
  exp_t *ep;
   
  ep = cdr(lp);
  if (isnull(ep) || !chkpars(car(ep)) || isnull(cdr(ep)))
	return everr("syntax error in", lp);
  return proc(func(car(ep), cons(atom("begin"), cdr(ep)), envp));
}

/* Verify if the expression represents function's parameters */
static int
chkpars(exp_t *ep)
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

/* Eval a let expression */
static exp_t *
evlet(exp_t *ep, env_t *envp)
{
  exp_t *lp, *plst, *vlst, *op;

  if (isnull(cdr(ep)) || isnull(cdr(cdr(ep))))
	return everr("syntax error", ep);
  plst = vlst = null;
  for (lp = car(cdr(ep)); ispair(lp); lp = cdr(lp)) {
	if (!islist(car(lp)) || !chknum(car(lp), 2))
	  return everr("syntax error", ep);
	plst = cons(car(car(lp)), plst);
	vlst = cons(car(cdr(car(lp))), vlst);
  }
  if (!isnull(lp))
	return everr("should be a list of bindings", lp);
  op = cons(atom("lambda"), cons(plst, cdr(cdr(ep))));
  return eval(cons(op, vlst), envp);
}

/* Eval a compound expression */
static exp_t *evmap(exp_t *, env_t *);

static exp_t *
evapply(exp_t *ep, env_t *envp)
{
  exp_t *op;
  exp_t *args;
  exp_t *parp;
  exp_t *blist;	/* binding list */

  if (!isproc(op = eval(car(ep), envp)))
	return (op ? everr("expression is not a procedure", car(ep)): NULL);
  if ((args = evmap(cdr(ep), envp)) == NULL)
	return NULL;
  if (procp(op)->tp == PRIM)	/* primitive */
	return primp(op)(args, envp);
  
  /* function */
  for (parp = fpar(op), blist = null ; !isatom(parp);
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
static exp_t *
evmap(exp_t *lp, env_t *envp)
{
  exp_t *ep, *res, *memp;

  for (memp = null; !isnull(lp); lp = cdr(lp)) {
	if ((ep = eval(car(lp), envp)) == NULL)	/* an error occured */
	  return NULL;
	memp = cons(ep, memp);
  }
  /* reverse the list */
  for (res = null; !isnull(memp); memp = cdr(memp))
	res = cons(car(memp), res);
  return res;
}
