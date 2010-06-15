#include "extern.h"
#include "exp.h"
#include "env.h"
#include "eval.h"
#include "type.h"

const excpt_t eval_error = { "eval" };
const excpt_t syntax_error = { "syntax" };

/* Represents an evaluation procedure. */
typedef struct evproc {
        exp_t *(*eval)();
        void **argv;
} evproc_t;

static inline evproc_t *
nevproc(exp_t *(*eval)(), void **argv)
{
        evproc_t *epp;

        NEW(epp);
        epp->eval = eval;
        epp->argv = argv;
        return epp;
}

static exp_t *evself(void **, env_t *);
static exp_t *evvar(void **, env_t *);
static exp_t *evdef(void **, env_t *);

static evproc_t *anself(exp_t *);
static evproc_t *anvar(exp_t *);
static evproc_t *anquote(exp_t *);
static evproc_t *andef(exp_t *);

/*
 * Check the syntax of the expression and return a corresponding
 * evaluation procedure.
 */
static evproc_t *
analyze(exp_t *ep)
{
        if (isself(ep))
                return anself(ep);
        else if (isvar(ep))
                return anvar(ep);
        else if (isquote(ep))
                return anquote(ep);
        else if (isdef(ep))
                return andef(ep);
        else
                anerr("bad syntax in", ep);
        return NULL;            /* not reached */
}

/* Evaluate the expression in the environment. */
exp_t *
eval(exp_t *exp, env_t *envp)
{
        evproc_t *epp;

        epp = analyze(exp);
        return epp->eval(epp->argv, envp);
}

/*
 * Syntax analyzer procedures.
 */

/* Check that the expression is a list of length n. */
static inline void
chklst(exp_t *lp, int n)
{
        exp_t *ep;

        for (ep = lp; n-- && ispair(ep); ep = cdr(ep))
                ;
        if (n != -1 || !isnull(ep))
                anerr("bad syntax in", lp);
}

/* Analyze a self-evaluating expression. */
static evproc_t *
anself(exp_t *ep)
{
        void **argv;

        NEW(argv);
        argv[0] = (void *)ep;
        return nevproc(evself, argv);
}

/* Analyze a variable. */
static evproc_t *
anvar(exp_t *ep)
{
        void **argv;

        NEW(argv);
        argv[0] = (void *)ep;
        return nevproc(evvar, argv);
}

/* Analyze the syntax of a quoted expression. */
static evproc_t *
anquote(exp_t *ep)
{
        chklst(ep, 2);
        return nevproc(evself, (void *)&cadr(ep));
}

static evproc_t *
andef(exp_t *ep)
{
        symb_t *var;
        evproc_t *vproc;
        exp_t *lst;
        void **argv;

        if (isnull(cdr(ep)) || isatom(cadr(ep)))
                chklst(ep, 3);
        lst = cdr(ep);
        if (ispair(ep = car(lst))) { /* lambda shortcut */
                if (!issym(car(ep)))
                        anerr("should be a symbol", cdr(lst));
                var = symp(car(ep));
                vproc = analyze(cons(atom(keywords[LAMBDA]),
                                   cons(cdr(ep), cdr(lst))));
        } else if (issym(ep)) {
                var = symp(ep);
                vproc = analyze(cadr(lst));
        } else
                anerr("the expression couldn't be defined", car(lst));
        argv = smalloc(2*sizeof(*argv));
        argv[0] = (void *)var;
        argv[1] = (void *)vproc;
        return nevproc(evdef, argv);
}

/*
 * Evaluation procedures.
 */

static exp_t *
evself(void **args, env_t *envp)
{
        return *args;
}

/* Return the value of a variable if any. */
static exp_t *
evvar(void **argv, env_t *envp)
{
        struct nlist *np;
        exp_t *var;

        var = (exp_t *)*argv;
        if ((np = lookup(symp(var), envp)) == NULL)
                everr("unbound variable", var);
        return np->defn;
}

/* Evaluate a define expression */
static exp_t *
evdef(void **argv, env_t *envp)
{
        symb_t *var;
        evproc_t *vproc;
        exp_t *val;

        var = (symb_t *)argv[0];
        vproc = (evproc_t *)argv[1];
        val = vproc->eval(vproc->argv, envp);
        if (type(val) == PROC && label(val) == NULL)
                label(val) = strtoatm(var); /* label anonymous procedure */
        install(var, val, envp);

        return NULL;
}

/* Evaluate a set! expression. */
static exp_t *
evset(exp_t *ep, env_t *envp)
{
        exp_t *var;
        exp_t *val;
        struct nlist *np;

        chklst(ep, 3);
        if (!issym(var = cadr(ep)))
                everr("should be a symbol", var);
        val = eval(caddr(ep), envp);
        if (!(np = lookup(symp(var), envp)))
                everr("unbound variable", var);
        np->defn = val;
        return NULL;
}

/* Set an expression to a new value. */
enum place { CAR, CDR };

static exp_t *
set(exp_t *ep, env_t *envp, enum place place)
{
        exp_t *var;
        exp_t *val;

        chklst(ep, 3);
        var = eval(cadr(ep), envp);
        val = eval(caddr(ep), envp);
        if (!ispair(var))
                everr("should be a pair", cadr(ep));
        else if (place == CAR)
                car(var) = val;
        else
                cdr(var) = val;
        return NULL;
}

/* Evaluate a set-car! expression */
static exp_t *
evsetcar(exp_t *ep, env_t *envp)
{
        return set(ep, envp, CAR);
}

/* Evaluate a set-cdr! expression */
static exp_t *
evsetcdr(exp_t *ep, env_t *envp)
{
        return set(ep, envp, CDR);
}

/* Evaluate an if expression */
static exp_t *
evif(exp_t *ep, env_t *envp)
{
        exp_t *res;

        chklst(ep, 4);
        ep = cdr(ep);
        res = (iseq(false, eval(car(ep), envp)) ? caddr(ep): cadr(ep));
        return eval(res, envp);
}

/* Evaluate a begin expression */
static exp_t *
evbegin(exp_t *ep, env_t *envp)
{
        exp_t *rv = NULL;

        for (ep = cdr(ep); !isnull(ep); ep = cdr(ep))
                rv = eval(car(ep), envp);
        return rv;
}

/* Evaluate a cond expression */
static exp_t *
evcond(exp_t *ep, env_t *envp)
{
        exp_t *_else_ = atom("else"), *arrow = atom("=>");
        exp_t *cl, *clauses, *b;

        /* Check the syntax. */
        for (clauses = cdr(ep); !isnull(clauses); clauses = cdr(clauses)) {
                if (!islist(cl = car(clauses)))
                        everr("should be a list", cl);
                if (iseq(_else_, car(cl)) && !isnull(cdr(clauses)))
                        everr("else clause must be last", ep);
                if (iseq(arrow, cadr(cl)))
                        if (iseq(_else_, car(cl)))
                                everr("illegal use of arrow", cl);
                        else if (!isnull(cdddr(cl)))
                                everr("bad clause form", cl);
        }

        /* Evaluate the expression. */
        for (clauses = cdr(ep); !isnull(clauses); clauses = cdr(clauses))
                if (iseq(_else_, car(cl)) ||
                    !iseq(false, b = eval(car(cl), envp)))
                        return iseq(arrow, cadr(cl)) ?
                                apply(eval(caddr(cl), envp),
                                      cons(b, null),
                                      envp) :
                                eval(cons(atom("begin"), cdr(cl)), envp);

        return null;
}

/* Evaluate an and expression */
static exp_t *
evand(exp_t *ep, env_t *envp)
{
        exp_t *res = true;

        for (ep = cdr(ep); !isnull(ep) && !iseq(false, res); ep = cdr(ep))
                res = eval(car(ep), envp);
        return res;
}

/* Evaluate an or expression */
static exp_t *
evor(exp_t *ep, env_t *envp)
{
        exp_t *res = false;

        for (ep = cdr(ep); !isnull(ep) && iseq(false, res); ep = cdr(ep))
                res = eval(car(ep), envp);
        return res;
}

/* Evaluate a lambda expression */
static int chkpars(exp_t *);

static exp_t *
evlambda(exp_t *lp, env_t *envp)
{
        exp_t *ep;

        ep = cdr(lp);
        if (isnull(ep) || !chkpars(car(ep)) || isnull(cdr(ep)))
                everr("syntax error in", lp);
        return proc(func(car(ep), cons(atom("begin"), cdr(ep)), envp));
}

/* Verify if the expression represents function's parameters */
static int
chkpars(exp_t *ep)
{
        if (!ispair(ep))
                return isatom(ep);
        for (; !isatom(ep); ep = cdr(ep))
                if (!issym(car(ep))) {
                        warnx("should be a symbol %s: ", tostr(car(ep)));
                        return 0;
                }
        return 1;
}

/* Eval a let expression */
static exp_t *
evlet(exp_t *ep, env_t *envp)
{
        exp_t *binds, *name, *body, *plst, *vlst, *op;

        if (isnull(cdr(ep)) || isnull(cddr(ep)))
                everr("syntax error", ep);
        if (issym(cadr(ep))) {
                name = cadr(ep);
                binds = caddr(ep);
                body = cdddr(ep);
        } else {
                name = NULL;
                binds = cadr(ep);
                body = cddr(ep);
        }

        for (plst = vlst = null; ispair(binds); binds = cdr(binds))
                if (issym(car(binds))) {
                        plst = cons(car(binds), plst);
                        vlst = cons(null, vlst);
                } else if (ispair(car(binds)) &&
                           ispair(cdar(binds)) &&
                           isnull(cddar(binds))) {
                        plst = cons(caar(binds), plst);
                        vlst = cons(cadar(binds), vlst);
                } else
                        everr("syntax error", ep);

        if (!isnull(binds))
                everr("should be a list of bindings", binds);
        op = cons(atom("lambda"), cons(nreverse(plst), body));
        if (name != NULL) {     /* named let */
                eval(cons(atom("define"),
                          cons(name, cons(op, null))),
                     envp);
                op = name;
        }

        return eval(cons(op, nreverse(vlst)), envp);
}

/* Apply a procedure to its arguments */
exp_t *
apply(exp_t *op, exp_t* args, env_t *envp)
{
        exp_t *parp;
        exp_t *blist; /* binding list */

        if (!isproc(op))
                everr("expression is not a procedure", op);
        if (procp(op)->tp == PRIM)    /* primitive */
                return primp(op)(args, envp);

        /* function */
        for (parp = fpar(op), blist = null ; !isatom(parp);
             parp = cdr(parp), args = cdr(args)) {
                if (isnull(args))
                        everr("too few arguments provided to", op);
                blist = cons(cons(car(parp), car(args)), blist);
        }
        if (!isnull(parp))      /* variable length arguments */
                blist = cons(cons(parp, args), blist);
        else if (!isnull(args))
                everr("too many arguments provided to", op);
        return eval(fbody(op), extenv(blist, fenv(op)));
}

/* Return a list of evaluated expressions */
static exp_t *
evmap(exp_t *lp, env_t *envp)
{
        exp_t *res;

        for (res = null; !isnull(lp); lp = cdr(lp))
                res = cons(eval(car(lp), envp), res);
        return nreverse(res);
}
