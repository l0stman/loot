#ifndef PRIM_H
#define PRIM_H

#define toint(s)		strtol((symp(s)), NULL, 10)
#define apply(op, a, b)	inttostr(toint(a) op toint(b))

extern int load(char *, struct env *);
extern void instprim(struct env *);

#endif /* !PRIM_H */
