#include "extern.h"
#include "exp.h"
#include "env.h"

exp_t *false;
exp_t *true;
exp_t *null;

static struct {
        exp_t **ep;
        symb_t *name;
} cst[] = {
        {&false, "#f"},
        {&true, "#t"},
        {&null, "()"}
};

#define X(k, s)	s
void *keywords[] = { KEYWORDS };
#undef	X

/* Initiate the variables and install the constants in the
   environment */
void
instcst(struct env *envp)
{
        register int i;

        for (i = 0; i < NELEMS(cst); i++) {
                *cst[i].ep = atom(cst[i].name);
                install(cst[i].name, *cst[i].ep, envp);
        }
}

/* Transform the strings in keywords into symbol expressions. */
void
initkeys(void)
{
        register int i;

        for (i = 0; i < NELEMS(keywords); i++)
                keywords[i] = atom(keywords[i]);
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
                size_t len = strlen(cdr)-2; /* ignore parenthesis */
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

        snprintf(buf, FMAXDIG, "%e", flt(ep));
        return buf;
}

/* Return a string representing a rational. */
static char *
rtostr(const exp_t *ep)
{
        char *buf = xalloc(2*IMAXDIG+2);

        sprintf(buf, "%d/%d", num(ep), den(ep));
        return buf;
}

/* Return a string representing a fixnum. */
static char *
fxntostr(const exp_t *ep)
{
        char *buf = xalloc(IMAXDIG+1);

        sprintf(buf, "%d", fixnum(ep));
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
        else if (isfxn(ep))
                return fxntostr(ep);
        else
                err_quit("tostr: unknown expression");
        return NULL;
}

#define SIGN(x) ((x) < 0 ? -1 : 1)
#define ABS(x)  ((x) == INT_MIN ? INT_MAX + 1U : ((x) < 0 ? -x : x))

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
nrat(int num, int den)
{
        exp_t *ep;
        unsigned int d, g;

        assert(den != 0);
        if (num == 0)
                return nfixnum(0);

        d = ABS(den);
        g = gcd(ABS(num), d);
        num = SIGN(den) * num/(int)g;
        d /= g;

        if (d == 1)
                return nfixnum(num);
        NEW(ep);
        type(ep) = RAT;
        NEW(ratp(ep));
        num(ep) = num;
        den(ep) = d;

        return ep;
}
