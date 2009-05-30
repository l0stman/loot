#include "loot.h"
#include "exp.h"
#include "env.h"
#include "eval.h"
#include "type.h"

static struct exp *everr(char *msg, struct exp *);
static struct exp *evdef(struct exp *, struct env *);
static struct exp *evvar(struct exp *, struct env *);
static struct exp *evquote(struct exp *);
static struct exp *evif(struct exp *, struct env *);
static struct exp *evbegin(struct exp *, struct env *);
static struct exp *evcond(struct exp *, struct env *);

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
  else
	return everr("unknown expression", ep);
}

/* Print the message and return a null pointer */
static struct exp *
everr(char *msg, struct exp *ep)
{
  char *s;
  
  warnx("%s: %s", msg, s = tostr(ep));
  free(s);
  return NULL;
}

/* Eval a define expression */
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
bind(char **varp, struct exp **valp, struct exp *ep)
{
  if (issym(car(ep))) {
	*varp = symp(car(ep));
	*valp = car(cdr(ep));
	return 1;
  }
  everr("the expression couldn't be defined", car(ep));
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

/* Eval an if expression */
static struct exp *
evif(struct exp *ep, struct env *envp)
{
  struct exp *b, *res;
  
  if (!chknum(ep, 4))
	return NULL;
  ep = cdr(ep);
  b = eval(car(ep), envp);
  res = (iseq(&false, b) ? car(cdr(cdr(ep))): car(cdr(ep)));
  return eval(res, envp);
}

/* Eval a begin expression */
static struct exp *
evbegin(struct exp *ep, struct env *envp)
{
  struct exp *form = NULL;

  for (ep = cdr(ep); !isnull(ep); ep = cdr(ep))
	form = eval(car(ep), envp);
  return form;
}

/* Eval a cond expression */
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
