#include "loot.h"
#include "exp.h"
#include "type.h"
#include "prim.h"

struct exp *prim_add(struct exp *);
struct proc proc_add = { PRIM, "+", {prim_add} };

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
  &proc_add, &proc_eq, &proc_cons, &proc_car, &proc_cdr
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

/* Return the sum of the expressions */
struct exp *
prim_add(struct exp *args)
{
  char buf[MAXDIG+1];
  int sum, argc;

  for (sum = 0, argc = 1; !isnull(args); args = cdr(args), argc++)
	if (!isnum(car(args))) {
	  warnx("+: the argument number %d isn't a number", argc);
	  return NULL;
	} else
	  sum += atoi(symp(car(args)));
  
  snprintf(buf, MAXDIG+1, "%d", sum);
  return atom(buf);
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
