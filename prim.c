#include <math.h>

#include "loot.h"
#include "exp.h"
#include "type.h"
#include "prim.h"
#include "reader.h"
#include "env.h"
#include "eval.h"
#include "parser.h"

static exp_t *prim_add(exp_t *);
static exp_t *prim_sub(exp_t *);
static exp_t *prim_prod(exp_t *);
static exp_t *prim_div(exp_t *);
static exp_t *prim_eq(exp_t *);
static exp_t *prim_sym(exp_t *);
static exp_t *prim_pair(exp_t *);
static exp_t *prim_numeq(exp_t *);
static exp_t *prim_lt(exp_t *);
static exp_t *prim_gt(exp_t *);
static exp_t *prim_isnum(exp_t *);
static exp_t *prim_cons(exp_t *);
static exp_t *prim_car(exp_t *);
static exp_t *prim_cdr(exp_t *);
static exp_t *prim_apply(exp_t *, env_t *);
static exp_t *prim_load(exp_t *, env_t *);
static exp_t *prim_sin(exp_t *);
static exp_t *prim_cos(exp_t *);
static exp_t *prim_tan(exp_t *);
static exp_t *prim_atan(exp_t *);
static exp_t *prim_log(exp_t *);
static exp_t *prim_exp(exp_t *);
static exp_t *prim_pow(exp_t *);

/* List of primitive procedures */
static struct {
  char *n;
  exp_t *(*pp)();
} plst[] = {
  /* arimthmetic */
  {"+", prim_add},
  {"-", prim_sub},
  {"*", prim_prod},
  {"/", prim_div},
  /* pair */
  {"cons", prim_cons},
  {"car", prim_car},
  {"cdr", prim_cdr},
  /* test */
  {"eq?", prim_eq},
  {"symbol?", prim_sym},
  {"pair?", prim_pair},
  {"=", prim_numeq},
  {"<", prim_lt},
  {">", prim_gt},
  {"number?", prim_isnum},
  /* math */
  {"sin", prim_sin},
  {"cos", prim_cos},
  {"tan", prim_tan},
  {"atan", prim_atan},
  {"log", prim_log},
  {"exp", prim_exp},
  {"expt", prim_pow},
  /* misc */
  {"apply", prim_apply},
  {"load", prim_load},
};

/* Install the primitive procedures in the environment */
void
instprim(env_t *envp)
{
  int i;

  for (i = 0; i < NELEMS(plst); i++)
	install(plst[i].n, proc(prim(plst[i].n, plst[i].pp)), envp);
}

/* Print the expression to the standard outupt */
static inline void
print(exp_t *ep)
{
  char *s;

  printf("%s %s\n", OUTPR, s = tostr(ep));
  free(s);
}

/* Evaluate all the expressions in the file */
int
load(char *path, env_t *envp)
{
  FILE *fp;
  buf_t *bp;
  exp_t *ep;
  
  if (path != NULL) {
	if ((fp = fopen(path, "r")) == NULL) {
	  warn("Can't open file %s", path);
	  return 1;
	}
  } else
	fp = stdin;
	  
  while ((bp = read(fp)) != NULL) {
	ep = eval(parse(bp->buf, bp->len), envp);
	if (inter && ep != NULL)
	  print(ep);
	bfree(bp);
  }
  fclose(fp);
  return 0;
}

/* Check if the primitive has the right number of arguments */
static inline int
chkargs(char *name, exp_t *args, int n)
{
  static char msg[BUFSIZ];
  exp_t *ep = args;
  
  while (n && !isnull(ep))
	n--, ep = cdr(ep);
  if (n == 0 && isnull(ep))
	return 1;
  snprintf(msg, BUFSIZ, "%s : wrong number of arguments", name);
  everr(msg, args);
  return 0;
}

/* Return the accumulation of the expression combined with the
   procedure f */
static exp_t *
foldl(exp_t *(*f)(), exp_t *init, exp_t *lst)
{
  exp_t *acc;

  for (acc = init; !isnull(lst); lst = cdr(lst))
	if ((acc = f(acc, car(lst))) == NULL)
	  return NULL;
  return acc;
}

/* Return the sum of two expressions */
static exp_t *
add(exp_t *a1, exp_t *a2)
{
  long n1, n2, d1, d2;
  exp_t *res;
  
  CHKNUM(a1, "+");
  CHKNUM(a2, "+");

  if (isfloat(a1) || isfloat(a2))
	res = nfloat(VALUE(a1) + VALUE(a2));
  else {
	n1 = NUMER(a1), n2 = NUMER(a2);
	d1 = DENOM(a1), d2 = DENOM(a2);
	res = nrat(n1*d2 + n2*d1, d1 * d2);
  } 
  return res;
}

/* Return the sum of the expressions */
static exp_t *
prim_add(exp_t *args)
{
  return foldl(add, atom("0"), args);
}

/* Return the difference of two expressions */
static exp_t *
sub(exp_t *a1, exp_t *a2)
{
  long n1, n2, d1, d2;
  exp_t *res;
  
  CHKNUM(a1, "-");
  CHKNUM(a2, "-");

  if (isfloat(a1) || isfloat(a2))
	res = nfloat(VALUE(a1) - VALUE(a2));
  else {
	n1 = NUMER(a1), n2 = NUMER(a2);
	d1 = DENOM(a1), d2 = DENOM(a2);
	res = nrat(n1*d2 - n2*d1, d1 * d2);
  } 
  return res;
}

/* Return the cumulated substraction of the arguments */
static exp_t *
prim_sub(exp_t *args)
{
  if (isnull(args))
	return everr("- : need at least one argument, given", null);
  else
	return foldl(sub, car(args), cdr(args));
}

/* Return the product of two expressions */
static exp_t *
prod(exp_t *a1, exp_t *a2)
{
  exp_t *res;

  CHKNUM(a1, "*");
  CHKNUM(a2, "*");

  if (isfloat(a1) || isfloat(a2))
	res = nfloat(VALUE(a1) * VALUE(a2));
  else 
	res = nrat(NUMER(a1) * NUMER(a2), DENOM(a1) * DENOM(a2));
  return res;
}

/* Return the product of the expressions */
static exp_t *
prim_prod(exp_t *args)
{
  return foldl(prod, atom("1"), args);
}

/* Return the division of two expressions */
static exp_t *
divs(exp_t *a1, exp_t *a2)
{
  exp_t *res;

  CHKNUM(a1, "/");
  CHKNUM(a2, "/");

  if (VALUE(a2) == 0)
	return everr("/: argument is divided by zero -- ", a1);
  if (isfloat(a1) || isfloat(a2))
	res = nfloat(VALUE(a1) / VALUE(a2));
  else
	res = nrat(NUMER(a1) * DENOM(a2), NUMER(a2) * DENOM(a1));
  return res;
}

/* Return the division of the expressions */
static exp_t *
prim_div(exp_t *args)
{
  if (isnull(args))
	return everr("/: need at least one argument -- given", null);
  else if(isnull(cdr(args)))
	return divs(atom("1"), car(args));
  else
	return foldl(divs, car(args), cdr(args));
}
  
/* Test if two expressions occupy the same physical memory */
static exp_t *
prim_eq(exp_t *args)
{
  if (!chkargs("eq?", args, 2))
	return NULL;
  return iseq(car(args), car(cdr(args))) ? true : false;
}

/* Test if the expression is a symbol */
static exp_t *
prim_sym(exp_t *args)
{
  if (!chkargs("symbol?", args, 1))
	return NULL;
  return issym(car(args)) ? true : false;
}

/* Test if the expression is a pair */
static exp_t *
prim_pair(exp_t *args)
{
  if (!chkargs("pair?", args, 1))
	return NULL;
  return ispair(car(args)) ? true : false;
}

/* Test if two numbers are equals */
static exp_t *
prim_numeq(exp_t *args)
{
  CHKCMP(args, "=");
  return compare(==, car(args), car(cdr(args)));
}

/* Test if the first argument is less than the second one */
static exp_t *
prim_lt(exp_t *args)
{
  CHKCMP(args, "<");
  return compare(<, car(args), car(cdr(args)));
}

/* Test if the first argument is greater than the second one */
static exp_t *
prim_gt(exp_t *args)
{
  CHKCMP(args, ">");
  return compare(>, car(args), car(cdr(args)));
}

/* Test if the argument is a number */
static exp_t *
prim_isnum(exp_t *args)
{
  if (!chkargs("number?", args, 1))
	return NULL;
  return isnum(car(args)) ? true: false;
}

/* Return a pair of expression */
static exp_t *
prim_cons(exp_t *args)
{
  if (!chkargs("cons", args, 2))
	return NULL;
  return cons(car(args), car(cdr(args)));
}

/* Return the first element of a pair */
static exp_t *
prim_car(exp_t *args)
{
  if (!chkargs("car", args, 1))
	return NULL;
  if (!ispair(car(args))) {
	everr("car: the argument isn't a pair", car(args));
	return NULL;
  }
  return car(car(args));
}

/* Return the second element of a pair */
static exp_t *
prim_cdr(exp_t *args)
{
  if (!chkargs("cdr", args, 1))
	return NULL;
  if (!ispair(car(args))) {
	everr("cdr: the argument isn't a pair", car(args));
	return NULL;
  }
  return cdr(car(args));
}

/* Apply a procedure expression to a list of expressions */
static exp_t *
prim_apply(exp_t *args, env_t *envp)
{
  exp_t *op;
  exp_t *lst;
  
  if (isnull(args) || isnull(cdr(args)))
	return everr("apply: expects at least 2 arguments, given -- ", args);
  op = car(args);
  for (lst = null, args = cdr(args); !isnull(cdr(args)); args = cdr(args))
	lst = cons(car(args), lst);
  if (!islist(car(args)))
	return everr("apply: should be a proper list -- ", car(args));
  for (args = car(args); !isnull(lst); lst = cdr(lst))
	args = cons(car(lst), args);
  return apply(op, args, envp);
}
  
/* Evaluate the expressions inside the file pointed by ep */
static exp_t *
prim_load(exp_t *args, env_t *envp)
{
  char *path;
  int mode = inter;
  
  if (!chkargs("load", args, 1))
	return NULL;
  else if (!isstr(car(args)))
	return everr("load: should be a string", car(args));
  path = (char *)symp(car(args));
  /* dump the quotes around the path name */
  path = sstrndup(path+1, strlen(path+1)-1);
  
  inter = 0;	/* non interactive mode */
  load(path, envp);
  inter = mode;
  free(path);
  return NULL;
}

/* Return the sine of the expression */
static exp_t *
prim_sin(exp_t *args)
{
  CALL(sin, args);
}

/* Return the cosine of the expression */
static exp_t *
prim_cos(exp_t *args)
{
  CALL(cos, args);
}

/* Return the tangent of the expression */
static exp_t *
prim_tan(exp_t *args)
{
  CALL(tan, args);
}

/* Return the arc tangent of the expression */
static exp_t *
prim_atan(exp_t *args)
{
  CALL(atan, args);
}

/* Return the natural logarithm of the expression */
static exp_t *
prim_log(exp_t *args)
{
  double v;
  
  if (!chkargs("log", args, 1))
	return NULL;
  else if (!isnum(car(args)) || (v = VALUE(car(args))) <= 0)
	return everr("log : not a positive number", car(args));
  else
	return nfloat(log(v));
}

/* Return the base e exponential of the expression */
static exp_t *
prim_exp(exp_t *args)
{
  CALL(exp, args);
}

/* Return the value of the first argument to the exponent of the
   second one */
static exp_t *
prim_pow(exp_t *args)
{
  if (!chkargs("expt", args, 2))
	return NULL;
  CHKNUM(car(args), "expt");
  CHKNUM(car(cdr(args)), "expt");

  return nfloat(pow(VALUE(car(args)),
					VALUE(car(cdr(args)))));
}
