#ifndef PRIM_H
#define PRIM_H

#define toint(s)	strtol((symp(s)), NULL, 10)
#define VALUE(x)	(isint(x) ? toint(x): fvalue(x))
#define APPLY(op, x, y)							\
  ((type(x) != type(y))							\
   || isfloat(x) ?								\
   nfloat(VALUE(x) op VALUE(y)) :				\
   atom(inttostr(VALUE(x) op VALUE(y))))

extern int load(char *, struct env *);
extern void instprim(struct env *);

#endif /* !PRIM_H */
