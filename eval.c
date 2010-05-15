#include "loot.h"
#include "exp.h"
#include "env.h"
#include "eval.h"
#include "type.h"

static exp_t *evdef(exp_t *, env_t *);
static exp_t *evvar(exp_t *, env_t *);
static exp_t *evquote(exp_t *);
static exp_t *evif(exp_t *, env_t *);
static exp_t *evbegin(exp_t *, env_t *);
static exp_t *evcond(exp_t *, env_t *);
static exp_t *evlambda(exp_t *, env_t *);
static exp_t *evand(exp_t *, env_t *);
static exp_t *evor(exp_t *, env_t *);
static exp_t *evlet(exp_t *, env_t *);
static exp_t *evmap(exp_t *, env_t *);
static exp_t *evset(exp_t *, env_t *);
static exp_t *evsetcar(exp_t *, env_t *);
static exp_t *evsetcdr(exp_t *, env_t *);

/* Evaluate the expression in the environment. */
exp_t *
eval(exp_t *ep, env_t *envp)
{
        if (isself(ep))
                return ep;
        else if (isdef(ep))
                return evdef(ep, envp);
        else if (isvar(ep))
                return evvar(ep, envp);
        else if (isquote(ep))
                return evquote(ep);
        else if (isif(ep))
                return evif(ep, envp);
        else if (isbegin(ep))
                return evbegin(ep, envp);
        else if (iscond(ep))
                return evcond(ep, envp);
        else if (islambda(ep))
                return evlambda(ep, envp);
        else if (isand(ep))
                return evand(ep, envp);
        else if (isor(ep))
                return evor(ep, envp);
        else if (islet(ep))
                return evlet(ep, envp);
        else if (isset(ep))
                return evset(ep, envp);
        else if (issetcar(ep))
                return evsetcar(ep, envp);
        else if (issetcdr(ep))
                return evsetcdr(ep, envp);
        else if (islist(ep))  /* application */
                return apply(eval(car(ep), envp), evmap(cdr(ep), envp), envp);
        else
                return everr("unknown expression", ep);
}

static int chknum(exp_t *, int);

/* Evaluate a define expression */
static exp_t *
evdef(exp_t *ep, env_t *envp)
{
        const char *var;
        exp_t *val, *args;

        if ((isnull(cdr(ep)) || isatom(cadr(ep))) && !chknum(ep, 3))
                return NULL;
        args = cdr(ep);
        if (ispair(ep = car(args))) { /* lambda shortcut */
                if (!issym(car(ep)))
                        return everr("should be a symbol", car(ep));
                var = symp(car(ep));
                val = cons(atom("lambda"), cons(cdr(ep), cdr(args)));
        } else if (issym(ep)) {
                var = symp(ep);
                val = cadr(args);
        } else
                return everr("the expression couldn't be defined", car(args));

        if ((val = eval(val, envp)) != NULL) {
                if (type(val) == PROC && label(val) == NULL)
                        /* label anonymous procedure */
                        label(val) = strtoatm(var);
                install(var, val, envp);
        }
        return val;
}

/* Return true if the expression length is equal to n */
static int
chknum(exp_t *lp, int n)
{
        exp_t *ep;

        for (ep = lp; n-- && !isnull(ep); ep = cdr(ep))
                ;
        return (n == -1 && isnull(ep) ||
                (int)everr("wrong number of expressions", lp));
}

/* Evaluate a set! expression. */
static exp_t *
evset(exp_t *ep, env_t *envp)
{
        exp_t *var;
        exp_t *val;
        struct nlist *np;

        if (!chknum(ep, 3))
                return NULL;
        if (!issym(var = cadr(ep)))
                return everr("should be a symbol", var);
        if (!(val = eval(caddr(ep), envp)))
                return NULL;
        if (!(np = lookup(symp(var), envp)))
                return everr("unbound variable", var);
        np->defn = val;
        return val;
}

/* Set an expression to a new value. */
enum place { CAR, CDR };

static exp_t *
set(exp_t *ep, env_t *envp, enum place place)
{
        exp_t *var;
        exp_t *val;

        if (!chknum(ep, 3))
                return NULL;
        if (!(var = eval(cadr(ep), envp)) ||
            !(val = eval(caddr(ep), envp)))
                return NULL;
        if (!ispair(var))
                return everr("should be a pair", cadr(ep));
        else if (place == CAR)
                car(var) = val;
        else
                cdr(var) = val;
        return val;
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

/* Return the value of a variable if any */
static exp_t *
evvar(exp_t *ep, env_t *envp)
{
        struct nlist *np;

        if ((np = lookup(symp(ep), envp)) == NULL)
                return everr("unbound variable", ep);
        return np->defn;
}

/* Return the quoted expression */
static exp_t *
evquote(exp_t *ep)
{
        return (chknum(ep, 2) ? cadr(ep): NULL);
}

/* Evaluate an if expression */
static exp_t *
evif(exp_t *ep, env_t *envp)
{
        exp_t *b, *res;

        if (!chknum(ep, 4))
                return NULL;
        ep = cdr(ep);
        if ((b = eval(car(ep), envp)) == NULL)
                return NULL;
        res = (iseq(false, b) ? caddr(ep): cadr(ep));
        return eval(res, envp);
}

/* Evaluate a begin expression */
static exp_t *
evbegin(exp_t *ep, env_t *envp)
{
        exp_t *rv = NULL;

        for (ep = cdr(ep); !isnull(ep); ep = cdr(ep))
                if ((rv = eval(car(ep), envp)) == NULL)
                        break;  /* an error occurred */
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
                        return everr("should be a list", cl);
                if (iseq(_else_, car(cl)) && !isnull(cdr(clauses)))
                        return everr("else clause must be last", ep);
                if (iseq(arrow, cadr(cl)))
                        if (iseq(_else_, car(cl)))
                                return everr("illegal use of arrow", cl);
                        else if (!isnull(cdddr(cl)))
                                return everr("bad clause form", cl);
        }

        /* Evaluate the expression. */
        for (clauses = cdr(ep); !isnull(clauses); clauses = cdr(clauses)) {
                if (iseq(_else_, car(cl)) ||
                    (b = eval(car(cl), envp)) != NULL && !iseq(false, b))
                        return iseq(arrow, cadr(cl)) ?
                                apply(eval(caddr(cl), envp),
                                      cons(b, null),
                                      envp) :
                                eval(cons(atom("begin"), cdr(cl)), envp);
                if (b == NULL)        /* an error occurred */
                        return NULL;
        }
        return null;
}

/* Evaluate an and expression */
static exp_t *
evand(exp_t *ep, env_t *envp)
{
        exp_t *res = true;

        for (ep = cdr(ep); !isnull(ep) && !iseq(false, res); ep = cdr(ep))
                if ((res = eval(car(ep), envp)) == NULL)
                        return NULL;
        return res;
}

/* Evaluate an or expression */
static exp_t *
evor(exp_t *ep, env_t *envp)
{
        exp_t *res = false;

        for (ep = cdr(ep); !isnull(ep) && iseq(false, res); ep = cdr(ep))
                if ((res = eval(car(ep), envp)) == NULL)
                        return NULL;
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
                return everr("syntax error in", lp);
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
                        everr("should be a symbol", car(ep));
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
                return everr("syntax error", ep);
        if (issym(cadr(ep))) {
                name = cadr(ep);
                binds = caddr(ep);
                body = cdddr(ep);
        } else {
                name = NULL;
                binds = cadr(ep);
                body = cddr(ep);
        }

        plst = vlst = null;
        for (; ispair(binds); binds = cdr(binds))
                if (issym(car(binds))) {
                        plst = cons(car(binds), plst);
                        vlst = cons(null, vlst);
                } else if (islist(car(binds)) && chknum(car(binds), 2)) {
                        plst = cons(caar(binds), plst);
                        vlst = cons(cadar(binds), vlst);
                } else
                        return everr("syntax error", ep);

        if (!isnull(binds))
                return everr("should be a list of bindings", binds);
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

        if (op == NULL || args == NULL)
                return NULL;
        if (!isproc(op))
                return everr("expression is not a procedure", op);
        if (procp(op)->tp == PRIM)    /* primitive */
                return primp(op)(args, envp);

        /* function */
        for (parp = fpar(op), blist = null ; !isatom(parp);
             parp = cdr(parp), args = cdr(args)) {
                if (isnull(args))
                        return everr("too few arguments provided to", op);
                blist = cons(cons(car(parp), car(args)), blist);
        }
        if (!isnull(parp))      /* variable length arguments */
                blist = cons(cons(parp, args), blist);
        else if (!isnull(args))
                return everr("too many arguments provided to", op);
        return eval(fbody(op), extenv(blist, fenv(op)));
}

/* Return a list of evaluated expressions */
static exp_t *
evmap(exp_t *lp, env_t *envp)
{
        exp_t *arg, *res;

        for (res = null; !isnull(lp); lp = cdr(lp)) {
                if ((arg = eval(car(lp), envp)) == NULL) /* an error occured */
                        return NULL;
                res = cons(arg, res);
        }
        return nreverse(res);
}
