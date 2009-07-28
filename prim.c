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
static exp_t *prim_eq(exp_t *);
static exp_t *prim_sym(exp_t *);
static exp_t *prim_pair(exp_t *);
static exp_t *prim_numeq(exp_t *);
static exp_t *prim_lt(exp_t *);
static exp_t *prim_gt(exp_t *);
static exp_t *prim_cons(exp_t *);
static exp_t *prim_car(exp_t *);
static exp_t *prim_cdr(exp_t *);
static exp_t *prim_eval(exp_t *, struct env *);
static exp_t *prim_load(exp_t *, struct env *);

/* List of primitive procedures */
static struct {
  char *n;
  exp_t *(*pp)();
} plst[] = {
  /* arimthmetic */
  {"+", prim_add},
  {"-", prim_sub},
  {"*", prim_prod},
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
  /* misc */
  {"eval", prim_eval},
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

/* inttostr: returns a string representing an integer */
static const char *
inttostr(long n)
{
  static char buf[MAXDIG+1];
  char *s = buf + sizeof(buf);
  unsigned long m;
  
  if (n == LONG_MIN)
	m = LONG_MAX + 1UL;
  else if (n < 0)
	m = -n;
  else
	m = n;
  do
	*--s = m%10 + '0';
  while ((m /= 10) > 0);
  if (n < 0)
	*--s = '-';
  return s;
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

/* Apply f to the elements of lst to built a result */
static exp_t *
foldl(exp_t *(*f)(), exp_t *init, exp_t *lst)
{
  exp_t *ep;

  for (ep = init; !isnull(lst); lst = cdr(lst))
	if ((ep = f(ep, car(lst))) == NULL)
	  return NULL;
  return ep;
}

/* Return the sum of two expressions */
static exp_t *
add(exp_t *sum, exp_t *ep)
{
  if (!isnum(ep))
	return everr("+: not a number", ep);
  return APPLY(+, sum, ep);
}

/* Return the sum of the expressions */
static exp_t *
prim_add(exp_t *args)
{
  return foldl(add, atom("0"), args);
}

/* Return the difference of two expressions */
static exp_t *
sub(exp_t *sum, exp_t *ep)
{
  if (!isnum(ep))
	return everr("-: not a number", ep);
  return APPLY(-, sum, ep);
}

/* Return the cumulated substraction of the arguments */
static exp_t *
prim_sub(exp_t *args)
{
  if (isnull(args))
	return everr("- : need at least one argument, given", null);
  else if (!isnum(car(args)))
	return everr("- : not a number", car(args));
  else
	return foldl(sub, car(args), cdr(args));
}

/* Return the product of two expressions */
static exp_t *
prod(exp_t *prod, exp_t *ep)
{
  if (!isnum(ep))
	return everr("*: not a number", ep);
  return APPLY(*, prod, ep);
}

/* Return the product of the expressions */
static exp_t *
prim_prod(exp_t *args)
{
  return foldl(prod, atom("1"), args);
}

/* Test if two expressions occupy the same physical memory */
static exp_t *
prim_eq(exp_t *args)
{
  if (!chkargs("eq?", args, 2))
	return NULL;
  return atom(iseq(car(args), car(cdr(args))) ? "#t": "#f");
}

/* Test if the expression is a symbol */
static exp_t *
prim_sym(exp_t *args)
{
  if (!chkargs("symbol?", args, 1))
	return NULL;
  return atom(issym(car(args)) ? "#t": "#f");
}

/* Test if the expression is a pair */
static exp_t *
prim_pair(exp_t *args)
{
  if (!chkargs("pair?", args, 1))
	return NULL;
  return atom(ispair(car(args)) ? "#t": "#f");
}

/* Test if two numbers are equals */
static exp_t *
prim_numeq(exp_t *args)
{
  CHECK(=, args);
  return compare(==, car(args), car(cdr(args)));
}

/* Test if the first argument is less than the second one */
static exp_t *
prim_lt(exp_t *args)
{
  CHECK(<, args);
  return compare(<, car(args), car(cdr(args)));
}

/* Test if the first argument is greater than the second one */
static exp_t *
prim_gt(exp_t *args)
{
  CHECK(>, args);
  return compare(>, car(args), car(cdr(args)));
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
	warnx("car: the argument isn't a pair");
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
	warnx("cdr: the argument isn't a pair");
	return NULL;
  }
  return cdr(car(args));
}

/* Eval the expression */
static exp_t *
prim_eval(exp_t *args, env_t *envp)
{
  if (!chkargs("eval", args, 1))
	return NULL;
  return eval(car(args), envp);
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
			  
