#include "loot.h"
#include "exp.h"
#include "type.h"
#include "prim.h"

struct exp *prim_add(struct exp *);
struct proc proc_add = { PRIM, "+", {prim_add} };

struct exp *prim_sub(struct exp *);
struct proc proc_sub = { PRIM, "-", {prim_sub} };

struct exp *prim_prod(struct exp *);
struct proc proc_prod = { PRIM, "*", {prim_prod} };

struct exp *prim_eq(struct exp *);
struct proc proc_eq = { PRIM, "eq?", {prim_eq} };

struct exp *prim_cons(struct exp *);
struct proc proc_cons = { PRIM, "cons", {prim_cons} };

struct exp *prim_car(struct exp *);
struct proc proc_car = { PRIM, "car", {prim_car} };

struct exp *prim_cdr(struct exp *);
struct proc proc_cdr = { PRIM, "cdr", {prim_cdr} };

/* List of primitive procedures */
struct proc *primlist[] = {
  /* arithmetic */
  &proc_add, &proc_sub, &proc_prod,
  /* pair */
  &proc_cons, &proc_car, &proc_cdr,
  /* test */
  &proc_eq,
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
foldr(struct exp *(*f)(), struct exp *init, struct exp *lst)
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
  return foldr(add, atom("0"), args);
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
	return foldr(sub, car(args), cdr(args));
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
  return foldr(prod, atom("1"), args);
}

/* Test if two expressions occupy the same physical memory */
struct exp *
prim_eq(struct exp *args)
{
  if (!chkargs("eq?", args, 2))
	return NULL;
  return atom(iseq(car(args), car(cdr(args))) ? "#t": "#f");
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
