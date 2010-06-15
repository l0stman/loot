#ifndef ATOM_H
#define ATOM_H

typedef const char symb_t;

extern symb_t *strtoatm(const char *);
extern symb_t *inttoatm(long);
extern symb_t *natom(const char *, int);

#endif	/* !ATOM_H */
