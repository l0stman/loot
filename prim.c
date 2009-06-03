#include "loot.h"
#include "exp.h"
#include "type.h"
#include "prim.h"
#include "reader.h"
#include "eval.h"
#include "parser.h"

struct exp *prim_add(struct exp *);
struct proc proc_add = { PRIM, "+", {prim_add} };

struct exp *prim_sub(struct exp *);
struct proc proc_sub = { PRIM, "-", {prim_sub} };

struct exp *prim_prod(struct exp *);
struct proc proc_prod = { PRIM, "*", {prim_prod} };

struct exp *prim_eq(struct exp *);
struct proc proc_eq = { PRIM, "eq?", {prim_eq} };

struct exp *prim_sym(struct exp *);
struct proc proc_sym = { PRIM, "symbol?", {prim_sym} };

struct exp *prim_pair(struct exp *);
struct proc proc_pair = { PRIM, "pair?", {prim_pair} };

struct exp *prim_cons(struct exp *);
struct proc proc_cons = { PRIM, "cons", {prim_cons} };

struct exp *prim_car(struct exp *);
struct proc proc_car = { PRIM, "car", {prim_car} };

struct exp *prim_cdr(struct exp *);
struct proc proc_cdr = { PRIM, "cdr", {prim_cdr} };

struct exp *prim_eval(struct exp *, struct env *);
struct proc proc_eval = { PRIM, "eval", {prim_eval} };

struct exp *prim_load(struct exp *, struct env *);
struct proc proc_load = { PRIM, "load", {prim_load} };

/* List of primitive procedures */
struct proc *primlist[] = {
  /* arithmetic */
  &proc_add, &proc_sub, &proc_prod,
  /* pair */
  &proc_cons, &proc_car, &proc_cdr,
  /* test */
  &proc_eq, &proc_sym, &proc_pair,
  /* misc */
  &proc_eval, &proc_load,
};
size_t psiz = sizeof(primlist)/sizeof(primlist[0]);

/* Check if the primitive has the right number of arguments */
static __inline__ int
chkargs(char *name, struct exp *args, int n)
{
  for (; n && !isnull(args); n--, args = cdr(args))
	;
  if (n == 0 && isnull(args))
	return 1;
  warnx("%s: wrong number of arguments", name);
  return 0;
}

/* Apply f to the elements of lst to built a result */
static struct exp *
foldl(struct exp *(*f)(), struct exp *init, struct exp *lst)
{
  struct exp *ep;

  for (ep = init; !isnull(lst); lst = cdr(lst))
	if ((ep = f(ep, car(lst))) == NULL)
	  return NULL;
  return ep;
}

/* Return the sum of two expressions */
static struct exp *
add(struct exp *sum, struct exp *ep)
{
  char buf[MAXDIG+1];

  if (!isnum(ep))
	return everr("+: not a number", ep);
  snprintf(buf, MAXDIG+1, "%d", atoi(symp(sum))+atoi(symp(ep)));
  return atom(buf);
}

/* Return the sum of the expressions */
struct exp *
prim_add(struct exp *args)
{
  return foldl(add, atom("0"), args);
}

/* Return the difference of two expressions */
static struct exp *
sub(struct exp *sum, struct exp *ep)
{
  char buf[MAXDIG+1];

  if (!isnum(ep))
	return everr("-: not a number", ep);
  snprintf(buf, MAXDIG+1, "%d", atoi(symp(sum))-atoi(symp(ep)));
  return atom(buf);
}

/* Return the cumulated substraction of the arguments */
struct exp *
prim_sub(struct exp *args)
{
  if (isnull(args))
	return everr("- : need at least one argument, given", &null);
  else if (!isnum(car(args)))
	return everr("- : not a number", car(args));
  else
	return foldl(sub, car(args), cdr(args));
}

/* Return the product of two expressions */
struct exp *
prod(struct exp *prod, struct exp *ep)
{
  char buf[MAXDIG+1];

  if (!isnum(ep))
	return everr("*: not a number", ep);
  snprintf(buf, MAXDIG+1, "%d", atoi(symp(prod))*atoi(symp(ep)));
  return atom(buf);
}

/* Return the product of the expressions */
struct exp *
prim_prod(struct exp *args)
{
  return foldl(prod, atom("1"), args);
}

/* Test if two expressions occupy the same physical memory */
struct exp *
prim_eq(struct exp *args)
{
  if (!chkargs("eq?", args, 2))
	return NULL;
  return atom(iseq(car(args), car(cdr(args))) ? "#t": "#f");
}

/* Test if the expression is a symbol */
struct exp *
prim_sym(struct exp *args)
{
  if (!chkargs("symbol?", args, 1))
	return NULL;
  return atom(issym(car(args)) ? "#t": "#f");
}

/* Test if the expression is a pair */
struct exp *
prim_pair(struct exp *args)
{
  if (!chkargs("pair?", args, 1))
	return NULL;
  return atom(ispair(car(args)) ? "#t": "#f");
}

/* Return a pair of expression */
struct exp *
prim_cons(struct exp *args)
{
  if (!chkargs("cons", args, 2))
	return NULL;
  return cons(car(args), car(cdr(args)));
}

/* Return the first element of a pair */
struct exp *
prim_car(struct exp *args)
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
struct exp *
prim_cdr(struct exp *args)
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
struct exp *
prim_eval(struct exp *args, struct env *envp)
{
  if (!chkargs("eval", args, 1))
	return NULL;
  return eval(car(args), envp);
}

/* Print the expression to the standard outupt */
static __inline__ void
print(struct exp *ep)
{
  char *s;

  printf("%s %s\n", OUTPR, s = tostr(ep));
  free(s);
}

/* Evaluate all the expressions in the file */
int
load(char *path, struct env *envp)
{
  FILE *fp;
  struct buf *bp;
  struct exp *ep;
  
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

/* Evaluate the expressions inside the file pointed by ep */
struct exp *
prim_load(struct exp *args, struct env *envp)
{
  char *path;
  int mode = inter;
  
  if (!chkargs("load", args, 1))
	return NULL;
  else if (!isstr(car(args)))
	return everr("load: should be a string", car(args));
  path = symp(car(args));
  /* dump the quotes around the path name */
  path = sstrndup(path+1, strlen(path+1)-1);
  
  inter = 0;	/* non interactive mode */
  load(path, envp);
  inter = mode;
  free(path);
  return NULL;
}
