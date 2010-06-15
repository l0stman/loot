#ifndef ATOM_H
#define ATOM_H

typedef const char symb_t;

/* Index of keyword symbols in keywords. */
enum kindex {
        DEFINE,
        QUOTE,
        IF,
        BEGIN,
        COND,
        LAMBDA,
        AND,
        OR,
        LET,
        SET,
        SETCAR,
        SETCDR
};

extern symb_t *keywords[];
extern void initkeys(void);

extern symb_t *strtoatm(const char *);
extern symb_t *inttoatm(long);
extern symb_t *natom(const char *, int);

#endif	/* !ATOM_H */
