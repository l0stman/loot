#ifndef PRIM_H
#define PRIM_H

#define toint(s)	strtol((symp(s)), NULL, 10)
#define VALUE(x)	(isint(x) ? toint(x): fvalue(x))
#define compare(op, x, y)	(VALUE(x) op VALUE(y) ? true: false)

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
