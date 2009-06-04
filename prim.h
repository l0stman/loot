#ifndef PRIM_H
#define PRIM_H

/* maximum number of digits (plus sign) for a 32-bits integer */
#define MAXDIG	10	

extern proc_t *primlist[];
extern size_t psiz;

int load(char *, struct env *);
#endif /* !PRIM_H */
