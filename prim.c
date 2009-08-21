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
static exp_t *prim_load(exp_t *, env_t *);

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
  /* misc */
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
  for (; n && !isnull(args); n--, args = cdr(args))
	;
  if (n == 0 && isnull(args))
	return 1;
  warnx("%s: wrong number of arguments", name);
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
  else  if (israt(a1) || israt(a2)) {
	n1 = NUMER(a1), n2 = NUMER(a2);
	d1 = DENOM(a1), d2 = DENOM(a2);
	res = nrat(n1*d2 + n2*d1, d1 * d2);
  } else
	res = atom(inttoatm(atoint(a1) + atoint(a2)));
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
  else  if (israt(a1) || israt(a2)) {
	n1 = NUMER(a1), n2 = NUMER(a2);
	d1 = DENOM(a1), d2 = DENOM(a2);
	res = nrat(n1*d2 - n2*d1, d1 * d2);
  } else
	res = atom(inttoatm(atoint(a1) - atoint(a2)));
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
  else if (israt(a1) || israt(a2))
	res = nrat(NUMER(a1) * NUMER(a2), DENOM(a1) * DENOM(a2));
  else
	res = atom(inttoatm(atoint(a1) * atoint(a2)));
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
