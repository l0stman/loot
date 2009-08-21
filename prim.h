#ifndef PRIM_H
#define PRIM_H

#define toint(s)	strtol((s), NULL, 10)
#define atoint(s)	toint(symp(s))
#define rvalue(r)	((1.0 * toint(num(r))) / toint(den(r)))
#define VALUE(x)	(isint(x) ? atoint(x):					\
					 (israt(x) ? rvalue(x) : fvalue(x)))

#define compare(op, x, y)	(VALUE(x) op VALUE(y) ? true: false)

/* Check if the expression is a number */
#define CHKNUM(x, name) {						\
	if (!isnum(x))								\
	  return everr(name": not a number", x);	\
  } while (0)

/* Returns the numerator and denominator of a rational or an integer */
#define NUMER(x)	(israt(x) ? toint(num(x)) : atoint(x))
#define DENOM(x)	(israt(x) ? toint(den(x)) : 1)

#define APPLY(op, x, y)							\
  ((type(x) != type(y))							\
   || isfloat(x) ?								\
   nfloat(VALUE(x) op VALUE(y)) :				\
   atom(inttostr(VALUE(x) op VALUE(y))))

#define CHECK(op, args)	do {											\
	if (!chkargs(#op, args, 2))											\
	  return NULL;														\
	if (!isnum(car(args)) || !isnum(car(cdr(args))))					\
	  return everr(#op": arguments should be numbers, given -- ", args); \
  } while (0)

extern int load(char *, struct env *);
extern void instprim(struct env *);

#endif /* !PRIM_H */
