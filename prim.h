#ifndef PRIM_H
#define PRIM_H

#define VALUE(x)	(isint(x) ? fixnum(x):          \
                         (israt(x) ? ((double)num(x)/(den(x))) : flt(x)))

#define compare(op, x, y)       (VALUE(x) op VALUE(y) ? true: false)

/* Check if the expression is a number */
#define CHKNUM(x, name) do {                                            \
                if (!isnum(x))                                          \
                        everr(#name": not a number", (x));              \
        } while (0)

/* Returns the numerator and denominator of a rational or an integer */
#define NUMER(x)        (israt(x) ? num(x) : fixnum(x))
#define DENOM(x)        (israt(x) ? den(x) : 1)

/* Check the arguments for a comparison */
#define CHKCMP(args, name)      do {            \
                chkargs(name, args, 2);         \
                CHKNUM(car(args), name);        \
                CHKNUM(cadr(args), name);       \
        } while (0)

/* Call the procedure to a list of one argument */
#define CALL(proc, args)        do {                    \
                chkargs(#proc, args, 1);                \
                CHKNUM(car(args), #proc);               \
                return nfloat(proc(VALUE(car(args))));  \
        } while (0)

typedef enum mode { NINTER, INTER } mode_t;

extern int load(char *, mode_t);
extern void instprim(struct env *);

#endif /* !PRIM_H */
