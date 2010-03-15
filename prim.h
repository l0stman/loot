#ifndef PRIM_H
#define PRIM_H

#define toint(s)        strtol((s), NULL, 10)
#define atoint(s)       toint(symp(s))
#define rvalue(r)       ((1.0 * toint(num(r))) / toint(den(r)))
#define VALUE(x)        (isint(x) ? atoint(x):                  \
                         (israt(x) ? rvalue(x) : fvalue(x)))

#define compare(op, x, y)       (VALUE(x) op VALUE(y) ? true: false)

/* Check if the expression is a number */
#define CHKNUM(x, name) do {                                            \
                if (!isnum(x))                                          \
                        return everr(name": not a number", (x));        \
        } while (0)

/* Returns the numerator and denominator of a rational or an integer */
#define NUMER(x)        (israt(x) ? toint(num(x)) : atoint(x))
#define DENOM(x)        (israt(x) ? toint(den(x)) : 1)

/* Check the arguments for a comparison */
#define CHKCMP(args, name)      do {            \
                if (!chkargs(name, args, 2))    \
                        return NULL;            \
                CHKNUM(car(args), name);        \
                CHKNUM(cadr(args), name);   \
        } while (0)

/* Call the procedure to a list of one argument */
#define CALL(proc, args)        do {                    \
                if (!chkargs(#proc, args, 1))           \
                        return NULL;                    \
                CHKNUM(car(args), #proc);               \
                return nfloat(proc(VALUE(car(args))));  \
        } while (0)

extern int load(char *, struct env *);
extern void instprim(struct env *);

#endif /* !PRIM_H */
