#ifndef PRIM_H
#define PRIM_H

/* maximum number of digits (plus sign) for a 32-bits integer */
#define MAXDIG	10

exp_t *prim_add(exp_t *);
exp_t *prim_sub(exp_t *);
exp_t *prim_prod(exp_t *);
exp_t *prim_eq(exp_t *);
exp_t *prim_sym(exp_t *);
exp_t *prim_pair(exp_t *);
exp_t *prim_cons(exp_t *);
exp_t *prim_car(exp_t *);
exp_t *prim_cdr(exp_t *);
exp_t *prim_eval(exp_t *, struct env *);
exp_t *prim_load(exp_t *, struct env *);

int load(char *, struct env *);
#endif /* !PRIM_H */
