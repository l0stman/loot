#ifndef PRIM_H
#define PRIM_H

#define toint(s)		strtol((symp(s)), NULL, 10)
#define apply(op, a, b)	inttostr(toint(a) op toint(b))

/* maximum number of digits (plus sign) for a 128-bits integer */
#define MAXDIG	39

extern exp_t *prim_add(exp_t *);
extern exp_t *prim_sub(exp_t *);
extern exp_t *prim_prod(exp_t *);
extern exp_t *prim_eq(exp_t *);
extern exp_t *prim_sym(exp_t *);
extern exp_t *prim_pair(exp_t *);
extern exp_t *prim_cons(exp_t *);
extern exp_t *prim_car(exp_t *);
extern exp_t *prim_cdr(exp_t *);
extern exp_t *prim_eval(exp_t *, struct env *);
extern exp_t *prim_load(exp_t *, struct env *);

int load(char *, struct env *);
#endif /* !PRIM_H */
