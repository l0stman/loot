#include "extern.h"
#include "exp.h"
#include "env.h"

exp_t *false;
exp_t *true;
exp_t *null;

static struct {
        exp_t **ep;
        const char *name;
} cst[] = {
        {&false, "#f"},
        {&true, "#t"},
        {&null, "()"}
};

/* Initiate the variables and install the constants in the
   environment */
void
instcst(struct env *envp)
{
        int i;

        for (i = 0; i < NELEMS(cst); i++) {
                *cst[i].ep = atom(cst[i].name);
                install(cst[i].name, *cst[i].ep, envp);
        }
}

/* Return true if the two expressions occupy the same memory.*/
int
iseq(const exp_t *a, const exp_t *b)
{
        return symp(a) == symp(b);
}

/* Return a string representing an atom */
static char *
atmtostr(const exp_t *ep)
{
        char *s;
        size_t len;

        len = strlen(symp(ep))+1;
        s = xalloc(len);
        memcpy(s, symp(ep), len);
        return s;
}

/* Return a string representing a pair */
static char *
pairtostr(const exp_t *ep)
{
        char *buf, *car, *cdr;
        size_t size;

        car = tostr(car(ep));
        size = strlen(car)+3;
        if (isnull(cdr(ep))) {
                buf = xalloc(size);
                snprintf(buf, size, "(%s)", car);
                goto clean;
        }
        cdr = tostr(cdr(ep));
        if (ispair(cdr(ep))) {
                size_t len = strlen(cdr)-2;
                size += len+1;
                buf = xalloc(size);
                snprintf(buf, size, "(%s %.*s)", car, len, cdr+1);
        } else {
                size += strlen(cdr)+3;
                buf = xalloc(size);
                snprintf(buf, size, "(%s . %s)", car, cdr);
        }
        xfree(cdr);
clean:
        xfree(car);
        return buf;
}

/* Return a string representing a procedure */
static char *
proctostr(const exp_t *ep)
{
        static char pref[] = "#<procedure";
        char *buf;
        size_t size;

        size = sizeof(pref)+(label(ep) ? strlen(label(ep)) : 0)+2;
        buf = xalloc(size);
        if (label(ep))
                snprintf(buf, size, "%s:%s>", pref, label(ep));
        else
                snprintf(buf, size, "%s>", pref);
        return buf;
}

/* Return a string representing a float */
static char *
ftostr(const exp_t *ep)
{
        char *buf = xalloc(FMAXDIG);

        snprintf(buf, FMAXDIG, "%e", fvalue(ep));
        return buf;
}

/* Return a string representing a rational */
static char *
rtostr(const exp_t *ep)
{
        char *buf = xalloc(strlen(num(ep))+strlen(num(ep))+2);

        sprintf(buf, "%s/%s", num(ep), den(ep));
        return buf;
}

/* Return a string representing the expression */
char *
tostr(const exp_t *ep)
{
        if (isatom(ep))
                return atmtostr(ep);
        else if (ispair(ep))
                return pairtostr(ep);
        else if (isproc(ep))
                return proctostr(ep);
        else if (isfloat(ep))
                return ftostr(ep);
        else if (israt(ep))
                return rtostr(ep);
        else
                err_quit("tostr: unknown expression");
        return NULL;
}

#define SIGN(x) ((x) < 0 ? -1 : 1)
#define ABS(x)  ((x) == LONG_MIN ? LONG_MAX + 1UL : ((x) < 0 ? -x : x))

static inline unsigned long
gcd(unsigned long m, unsigned long n)
{
        unsigned long r;

        do {
                r = m - (m/n) * n;
                m = n;
                n = r;
        } while (n);

        return m;
}

/* Built a new rational number */
exp_t *
nrat(long num, long den)
{
        exp_t *ep;
        unsigned long d, g;

        assert(den != 0);
        if (num == 0)
                return atom("0");

        d = ABS(den);
        g = gcd(ABS(num), d);
        num = SIGN(den) * num/(long)g;
        d /= g;

        if (d == 1)
                return atom(inttoatm(num));
        NEW(ep);
        type(ep) = RAT;
        NEW(ratp(ep));
        num(ep) = inttoatm(num);
        den(ep) = inttoatm(d);

        return ep;
}
