#include "extern.h"
#include "exp.h"
#include "env.h"
#include "eval.h"
#include "type.h"

const excpt_t eval_error = { "eval" };
const excpt_t syntax_error = { "syntax" };

static exp_t *evself(exp_t **, env_t *);
static exp_t *evvar(exp_t **, env_t *);
static exp_t *evdef(void **, env_t *);
static exp_t *evif(evproc_t **, env_t *);
static exp_t *evbegin(evproc_t **, env_t *);
static exp_t *evlambda(void **, env_t *);
static exp_t *evapp(evproc_t **, env_t *);

static evproc_t *anself(exp_t *);
static evproc_t *anvar(exp_t *);
static evproc_t *anquote(exp_t *);
static evproc_t *andef(exp_t *);
static evproc_t *anif(exp_t *);
static evproc_t *anbegin(exp_t *);
static evproc_t *anlambda(exp_t *);
static evproc_t *anapp(exp_t *);

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
        else if (isif(ep))
                return anif(ep);
        else if (isbegin(ep))
                return anbegin(ep);
        else if (islambda(ep))
                return anlambda(ep);
        else if (ispair(ep))    /* application */
                return anapp(ep);
        else
                anerr("bad syntax in", ep);
        return NULL;            /* not reached */
}

#define EVPROC(epp, envp)	((epp)->eval((epp)->argv, envp))

/* Evaluate the expression in the environment. */
exp_t *
eval(exp_t *exp, env_t *envp)
{
        evproc_t *epp;

        epp = analyze(exp);
        return EVPROC(epp, envp);
}

/* Apply a procedure to its arguments. */
exp_t *
apply(exp_t *op, exp_t *args, env_t *envp)
{
        exp_t *pars;
        exp_t *binds;           /* binding list */

        if (!isproc(op))
                everr("expression is not a procedure", op);
        if (procp(op)->tp == PRIM) /* primitive */
                return primp(op)(args, envp);

        /* function */
        for (pars = fpar(op), binds = null;
             ispair(pars);
             pars = cdr(pars), args = cdr(args)) {
                if (isnull(args))
                        everr("too few arguments provided to", op);
                binds = cons(cons(car(pars), car(args)), binds);
        }
        if (!isnull(pars))      /* variable length arguments */
                binds = cons(cons(pars, args), binds);
        else if (!isnull(args))
                everr("too many arguments provided to", op);

        return EVPROC(fbody(op), extenv(binds, fenv(op)));
}

/* * * * * * * * * * * * * * * *
 * Syntax analyzer procedures. *
 * * * * * * * * * * * * * * * */

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

/* Analyze the syntax of a define expression. */
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
                        anerr("should be a symbol", car(ep));
                var = symp(car(ep));
                vproc = analyze(cons(keywords[LAMBDA],
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

/* Analyze the syntax of an if expression. */
static evproc_t *
anif(exp_t *ep)
{
        evproc_t **argv;

        chklst(ep, 4);
        ep = cdr(ep);
        argv = smalloc(3*sizeof(*argv));
        argv[0] = analyze(car(ep));
        argv[1] = analyze(cadr(ep));
        argv[2] = analyze(caddr(ep));

        return nevproc(evif, (void **)argv);
}

/* Analyze the syntax of a begin expression. */
static evproc_t *
anbegin(exp_t *ep)
{
        evproc_t **argv;
        exp_t *lp;
        register int size;

        if (isnull(lp = cdr(ep)))
                anerr("empty form", ep);
        for (size = 1; ispair(lp); lp = cdr(lp))
                size++;
        if (!isnull(lp))
                anerr("should be a list", ep);

        argv = smalloc(size*sizeof(*argv));
        for (size = 0, lp = cdr(ep); ispair(lp); lp = cdr(lp))
                argv[size++] = analyze(car(lp));
        argv[size] = NULL;

        return nevproc(evbegin, (void **)argv);
}

/* Analyze the syntax of a lambda expression. */
static evproc_t *
anlambda(exp_t *ep)
{
        void **argv;
        exp_t *lp, *p;

        if (isnull(cdr(ep)) || isnull(cddr(ep)))
                anerr("bad syntax in", ep);
        for (lp = cadr(ep); ispair(lp); lp = cdr(lp)) {
                if (!issym(car(lp)))
                        anerr("should be a symbol", car(lp));
                for (p = cdr(lp); ispair(p); p = cdr(p))
                        if (iseq(car(lp), car(p)))
                                anerr("duplicate symbol parameter", car(p));
        }
        if (!isnull(lp) && !issym(lp))
                 anerr("should be null or a symbol", lp);

        argv = smalloc(2*sizeof(*argv));
        argv[0] = (void *)cadr(ep);
        argv[1] = (void *)anbegin(cons(keywords[BEGIN], cddr(ep)));

        return nevproc(evlambda, argv);
}

/* Analyze the syntax of an application expression. */
static evproc_t *
anapp(exp_t *ep)
{
        void **argv;
        exp_t *p;
        register int size;

        for (size = 1, p = ep; ispair(p); p = cdr(p))
                ++size;
        if (!isnull(p))
                anerr("an application should be a list, given", ep);
        argv = smalloc(size*sizeof(*argv));
        argv[0] = (void *)size;
        for (size = 1, p = ep; ispair(p); p = cdr(p))
                argv[size++] = analyze(car(p));

        return nevproc(evapp, argv);
}

/* * * * * * * * * * * * * *
 * Evaluation procedures.  *
 * * * * * * * * * * * * * */

static exp_t *
evself(exp_t **args, env_t *envp)
{
        return *args;
}

/* Return the value of a variable if any. */
static exp_t *
evvar(exp_t **argv, env_t *envp)
{
        struct nlist *np;

        if ((np = lookup(symp(argv[0]), envp)) == NULL)
                everr("unbound variable", argv[0]);
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
        val = EVPROC(vproc, envp);
        if (val && type(val) == PROC && label(val) == NULL)
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
evif(evproc_t **argv, env_t *envp)
{
        evproc_t *pred, *res;

        pred = argv[0];
        res = (!iseq(false, EVPROC(pred, envp)) ? argv[1] : argv[2]);
        return EVPROC(res, envp);
}

/* Evaluate a begin expression */
static exp_t *
evbegin(evproc_t **argv, env_t *envp)
{
        assert(*argv);
        for (; *(argv+1); argv++)
                EVPROC(argv[0], envp);
        return EVPROC(argv[0], envp);
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
static exp_t *
evlambda(void **argv, env_t *envp)
{
        return nproc(func((exp_t *)argv[0], (evproc_t *)argv[1], envp));
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

/* Evaluate an application expression. */
static exp_t *
evapp(evproc_t **argv, env_t *envp)
{
        int argc, i;
        exp_t *args;

        argc = (int)argv[0];
        for (args = null, i = 2; i<argc; i++)
                args = cons(EVPROC(argv[i], envp), args);
        return apply(EVPROC(argv[1], envp), nreverse(args), envp);
}
