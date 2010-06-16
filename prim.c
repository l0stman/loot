#include <math.h>

#include "extern.h"
#include "exp.h"
#include "type.h"
#include "prim.h"
#include "reader.h"
#include "env.h"
#include "eval.h"
#include "parser.h"

static exp_t *prim_add(exp_t *);
static exp_t *prim_sub(exp_t *);
static exp_t *prim_prod(exp_t *);
static exp_t *prim_div(exp_t *);
static exp_t *prim_eq(exp_t *);
static exp_t *prim_sym(exp_t *);
static exp_t *prim_pair(exp_t *);
static exp_t *prim_numeq(exp_t *);
static exp_t *prim_lt(exp_t *);
static exp_t *prim_gt(exp_t *);
static exp_t *prim_isnum(exp_t *);
static exp_t *prim_cons(exp_t *);
static exp_t *prim_car(exp_t *);
static exp_t *prim_cdr(exp_t *);
static exp_t *prim_apply(exp_t *, env_t *);
static exp_t *prim_load(exp_t *);
static exp_t *prim_sin(exp_t *);
static exp_t *prim_cos(exp_t *);
static exp_t *prim_tan(exp_t *);
static exp_t *prim_atan(exp_t *);
static exp_t *prim_log(exp_t *);
static exp_t *prim_exp(exp_t *);
static exp_t *prim_pow(exp_t *);
static exp_t *prim_read(void);
static exp_t *prim_write(exp_t *);

/* List of primitive procedures */
static struct {
        char *n;
        exp_t *(*pp)();
} plst[] = {
        /* arimthmetic */
        {"+", prim_add},
        {"-", prim_sub},
        {"*", prim_prod},
        {"/", prim_div},
        /* pair */
        {"cons", prim_cons},
        {"car", prim_car},
        {"cdr", prim_cdr},
        /* test */
        {"eq?", prim_eq},
        {"symbol?", prim_sym},
        {"pair?", prim_pair},
        {"=", prim_numeq},
        {"<", prim_lt},
        {">", prim_gt},
        {"number?", prim_isnum},
        /* math */
        {"sin", prim_sin},
        {"cos", prim_cos},
        {"tan", prim_tan},
        {"atan", prim_atan},
        {"log", prim_log},
        {"exp", prim_exp},
        {"expt", prim_pow},
        /* I/O */
        {"write", prim_write},
        {"read", prim_read},
        /* misc */
        {"apply", prim_apply},
        {"load", prim_load},
};

/* Install the primitive procedures in the environment */
void
instprim(env_t *envp)
{
        int i;

        for (i = 0; i < NELEMS(plst); i++)
                install(plst[i].n, nproc(nprim(plst[i].n, plst[i].pp)), envp);
}

/* Evaluate all the expressions in the file */
int
load(char *path, mode_t isinter)
{
        FILE     *fp;
        buf_t    *bp;
        exp_t    *ep;
        char     *file = filename;
        exfram_t *es   = exstack;
        int       line = linenum;
        int	  rc   = 0;

        exstack = NULL;
        linenum = 1;
        if (path != NULL) {
                filename = basename(path);
                if ((fp = fopen(path, "r")) == NULL) {
                        warn("Can't open file %s", path);
                        rc = 1;
                        goto restore;
                }
        } else  {
                filename = NULL;
                fp = stdin;
        }

read:
        TRY
                if (isinter) {
                        printf("%s", INPR);
                        fflush(stdout);
                }
                if ((bp = read(fp)) == NULL) {
                        xfreeall();
                        fclose(fp);
                        goto restore;
                }
                ep = eval(parse(bp->buf, bp->len), globenv);
                if (isinter && ep != NULL) {
                        printf("%s%s\n", OUTPR, tostr(ep));
                        fflush(stdout);
                }
        WARN(read_error);
        WARN(syntax_error);
        WARN(eval_error);
        ENDTRY;

        xfreeall();
        goto read;
restore:
        filename = file;
        linenum = line;
        exstack = es;
        return rc;
}

/* Check if the primitive has the right number of arguments */
static inline void
chkargs(char *name, exp_t *args, int num)
{
        exp_t *ep;
        int n;

        for (ep = args, n = num; n-- && !isnull(ep); ep = cdr(ep))
                ;
        if (n != -1 || !isnull(ep))
                raise(&eval_error, filename, linenum,
                      "%s: expects %d arguments, given %s",
                      name, num, tostr(args));
}

/* Return the accumulation of the expression combined with the
   procedure f */
static exp_t *
foldl(exp_t *(*f)(), exp_t *init, exp_t *lst)
{
        exp_t *acc;

        for (acc = init; !isnull(lst); lst = cdr(lst))
                if ((acc = f(acc, car(lst))) == NULL)
                        return NULL;
        return acc;
}

/* Return the sum of two expressions */
static exp_t *
add(exp_t *a1, exp_t *a2)
{
        long n1, n2, d1, d2;
        exp_t *res;

        CHKNUM(a1, +);
        CHKNUM(a2, +);

        if (isfloat(a1) || isfloat(a2))
                res = nfloat(VALUE(a1) + VALUE(a2));
        else {
                n1 = NUMER(a1), n2 = NUMER(a2);
                d1 = DENOM(a1), d2 = DENOM(a2);
                res = nrat(n1*d2 + n2*d1, d1 * d2);
        }
        return res;
}

/* Return the sum of the expressions */
static exp_t *
prim_add(exp_t *args)
{
        return foldl(add, nfixnum(0), args);
}

/* Return the difference of two expressions */
static exp_t *
sub(exp_t *a1, exp_t *a2)
{
        long n1, n2, d1, d2;
        exp_t *res;

        CHKNUM(a1, -);
        CHKNUM(a2, -);

        if (isfloat(a1) || isfloat(a2))
                res = nfloat(VALUE(a1) - VALUE(a2));
        else {
                n1 = NUMER(a1), n2 = NUMER(a2);
                d1 = DENOM(a1), d2 = DENOM(a2);
                res = nrat(n1*d2 - n2*d1, d1 * d2);
        }
        return res;
}

/* Return the cumulated substraction of the arguments */
static exp_t *
prim_sub(exp_t *args)
{
        if (isnull(args))
                everr("- : need at least one argument, given", null);
        else if (isnull(cdr(args)))
                return sub(nfixnum(0), car(args));
        return foldl(sub, car(args), cdr(args));
}

/* Return the product of two expressions */
static exp_t *
prod(exp_t *a1, exp_t *a2)
{
        exp_t *res;

        CHKNUM(a1, *);
        CHKNUM(a2, *);

        if (isfloat(a1) || isfloat(a2))
                res = nfloat(VALUE(a1) * VALUE(a2));
        else
                res = nrat(NUMER(a1) * NUMER(a2), DENOM(a1) * DENOM(a2));
        return res;
}

/* Return the product of the expressions */
static exp_t *
prim_prod(exp_t *args)
{
        return foldl(prod, nfixnum(1), args);
}

/* Return the division of two expressions */
static exp_t *
divs(exp_t *a1, exp_t *a2)
{
        exp_t *res;

        CHKNUM(a1, /);
        CHKNUM(a2, /);

        if (VALUE(a2) == 0)
                everr("/: argument is divided by zero", a1);
        if (isfloat(a1) || isfloat(a2))
                res = nfloat(VALUE(a1) / VALUE(a2));
        else
                res = nrat(NUMER(a1) * DENOM(a2), NUMER(a2) * DENOM(a1));
        return res;
}

/* Return the division of the expressions */
static exp_t *
prim_div(exp_t *args)
{
        if (isnull(args))
                everr("/: need at least one argument -- given", null);
        else if(isnull(cdr(args)))
                return divs(nfixnum(1), car(args));
        return foldl(divs, car(args), cdr(args));
}

/* Test if two expressions occupy the same physical memory */
static exp_t *
prim_eq(exp_t *args)
{
        chkargs("eq?", args, 2);
        return iseq(car(args), cadr(args)) ? true : false;
}

/* Test if the expression is a symbol */
static exp_t *
prim_sym(exp_t *args)
{
        chkargs("symbol?", args, 1);
        return issym(car(args)) ? true : false;
}

/* Test if the expression is a pair */
static exp_t *
prim_pair(exp_t *args)
{
        chkargs("pair?", args, 1);
        return ispair(car(args)) ? true : false;
}

/* Test if two numbers are equals */
static exp_t *
prim_numeq(exp_t *args)
{
        CHKCMP(args, "=");
        return compare(==, car(args), cadr(args));
}

/* Test if the first argument is less than the second one */
static exp_t *
prim_lt(exp_t *args)
{
        CHKCMP(args, "<");
        return compare(<, car(args), cadr(args));
}

/* Test if the first argument is greater than the second one */
static exp_t *
prim_gt(exp_t *args)
{
        CHKCMP(args, ">");
        return compare(>, car(args), cadr(args));
}

/* Test if the argument is a number */
static exp_t *
prim_isnum(exp_t *args)
{
        chkargs("number?", args, 1);
        return isnum(car(args)) ? true: false;
}

/* Return a pair of expression */
static exp_t *
prim_cons(exp_t *args)
{
        chkargs("cons", args, 2);
        return cons(car(args), cadr(args));
}

/* Return the first element of a pair */
static exp_t *
prim_car(exp_t *args)
{
        chkargs("car", args, 1);
        if (!ispair(car(args)))
                everr("car: the argument isn't a pair", car(args));
        return caar(args);
}

/* Return the second element of a pair */
static exp_t *
prim_cdr(exp_t *args)
{
        chkargs("cdr", args, 1);
        if (!ispair(car(args)))
                everr("cdr: the argument isn't a pair", car(args));
        return cdar(args);
}

/* Apply a procedure expression to a list of expressions */
static exp_t *
prim_apply(exp_t *args, env_t *envp)
{
        exp_t *op, *prev, *last;

        if (isnull(args) || isnull(cdr(args)))
                everr("apply: expects at least 2 arguments, given", args);
        op = car(args);
        if (!isnull(last = cddr(args))) {
                for (prev = cdr(args); !isnull(cdr(last)); last = cdr(last))
                        prev = last;
                cdr(prev) = car(last);
                args = cdr(args);
        } else {
                last = cdr(args);
                args = car(last);
        }
        if (!islist(car(last)))
                everr("apply: should be a proper list", car(last));
        return apply(op, args, envp);
}

/* Evaluate the expressions inside the file pointed by ep */
static exp_t *
prim_load(exp_t *args)
{
        char *path;

        chkargs("load", args, 1);
        if (!isstr(car(args)))
                everr("load: should be a string", car(args));
        path = (char *)symp(car(args));
        /* dump the quotes around the path name */
        path = sstrndup(path+1, strlen(path+1)-1);
        load(path, NINTER);
        free(path);
        return NULL;
}

/* Return the sine of the expression */
static exp_t *
prim_sin(exp_t *args)
{
        CALL(sin, args);
}

/* Return the cosine of the expression */
static exp_t *
prim_cos(exp_t *args)
{
        CALL(cos, args);
}

/* Return the tangent of the expression */
static exp_t *
prim_tan(exp_t *args)
{
        CALL(tan, args);
}

/* Return the arc tangent of the expression */
static exp_t *
prim_atan(exp_t *args)
{
        CALL(atan, args);
}

/* Return the natural logarithm of the expression */
static exp_t *
prim_log(exp_t *args)
{
        double v;

        chkargs("log", args, 1);
        if (!isnum(car(args)) || (v = VALUE(car(args))) <= 0)
                everr("log : not a positive number", car(args));
        return nfloat(log(v));
}

/* Return the base e exponential of the expression */
static exp_t *
prim_exp(exp_t *args)
{
        CALL(exp, args);
}

/* Return the value of the first argument to the exponent of the
   second one */
static exp_t *
prim_pow(exp_t *args)
{
        exp_t *res, *b;
        long e;
        unsigned long u;

        chkargs("expt", args, 2);
        CHKNUM(car(args), expt);
        CHKNUM(cadr(args), expt);

        res = cadr(args);
        if (isint(res)) {
                e = VALUE(res);
                if (e == LONG_MIN)
                        u = LONG_MAX + 1UL;
                else if (e < 0)
                        u = -e;
                else
                        u = e;
                res = atom("1");
                b = car(args);
                while (u) {
                        if (u & 1) {
                                res = prod(res, b);
                                u--;
                        }     else {
                                b = prod(b, b);
                                u /= 2;
                        }
                }
                if (e < 0)
                        res = divs(atom("1"), res);
        } else
                res = nfloat(pow(VALUE(car(args)),
                                 VALUE(cadr(args))));
        return res;
}

/* Write the expression to the standard output. */
static exp_t *
prim_write(exp_t *args)
{
        chkargs("write", args, 1);
        printf("%s", tostr(car(args)));
        return NULL;
}

/* Read an expression from the standard input. */
static exp_t *
prim_read(void)
{
        buf_t *bp;

        bp = read(stdin);
        return parse(bp->buf, bp->len);
}
