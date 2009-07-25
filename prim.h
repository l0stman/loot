#ifndef PRIM_H
#define PRIM_H

#define toint(s)		strtol((symp(s)), NULL, 10)
#define apply(op, a, b)	inttostr(toint(a) op toint(b))

/* maximum number of digits (plus sign) for a 128-bits integer */
#define MAXDIG	39

extern int load(char *, struct env *);
extern void instprim(struct env *);

#endif /* !PRIM_H */
