#include "extern.h"
#include "exp.h"
#include "env.h"
#include "eval.h"
#include "type.h"

const excpt_t eval_error = { "eval" };
const excpt_t syntax_error = { "syntax" };
static enum place { CAR, CDR };
static enum logic { LAND, LOR };

static exp_t *evself(exp_t **, env_t *);
static exp_t *evvar(exp_t **, env_t *);
static exp_t *evdef(void **, env_t *);
static exp_t *evif(evproc_t **, env_t *);
static exp_t *evbegin(evproc_t **, env_t *);
static exp_t *evlambda(void **, env_t *);
static exp_t *evapp(evproc_t **, env_t *);
static exp_t *evcond(evproc_t **, env_t *);
static exp_t *evset(void **, env_t *);
static exp_t *evsetpair(evproc_t **, env_t *);
static exp_t *evor(evproc_t **, env_t *);
static exp_t *evand(evproc_t **, env_t *);

static evproc_t *anself(exp_t *);
static evproc_t *anvar(exp_t *);
static evproc_t *anquote(exp_t *);
static evproc_t *andef(exp_t *);
static evproc_t *anif(exp_t *);
static evproc_t *anbegin(exp_t *);
static evproc_t *anlambda(exp_t *);
static evproc_t *anapp(exp_t *);
static evproc_t *ancond(exp_t *);
static evproc_t *anset(exp_t *);
static evproc_t *ansetpair(exp_t *, enum place);
static evproc_t *anlogic(exp_t *, enum logic);

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
        else if (iscond(ep))
                return ancond(ep);
        else if (isset(ep))
                return anset(ep);
        else if (issetcar(ep))
                return ansetpair(ep, CAR);
        else if (issetcdr(ep))
                return ansetpair(ep, CDR);
        else if (isor(ep))
                return anlogic(ep, LOR);
        else if (isand(ep))
                return anlogic(ep, LAND);
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
apply(exp_t *op, exp_t *args)
{
        exp_t *pars;
        exp_t *binds;           /* binding list */

        if (!isproc(op))
                everr("expression is not a procedure", op);
        if (ptype(op) == PRIM) /* primitive */
                return primp(op)(args);

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

#define nlambda(pars, body)	(cons(keywords[LAMBDA], cons(pars, body)))

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
                vproc = analyze(nlambda(cdr(ep), cdr(lst)));
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
        register int argc;

        if (isnull(lp = cdr(ep)))
                anerr("empty form", ep);
        for (argc = 1; ispair(lp); lp = cdr(lp))
                argc++;
        if (!isnull(lp))
                anerr("should be a list", ep);

        argv = smalloc(argc*sizeof(*argv));
        argv[0] = (evproc_t *)argc;
        for (argc = 1, lp = cdr(ep); ispair(lp); lp = cdr(lp))
                argv[argc++] = analyze(car(lp));

        return nevproc(evbegin, (void **)argv);
}

#define nseq(ep)	(cons(keywords[BEGIN], ep))

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
        argv[1] = (void *)anbegin(nseq(cddr(ep)));

        return nevproc(evlambda, argv);
}

#define iselse(p)	(iseq(keywords[ELSE], (exp_t *)(p)))
#define isarrow(p)	(iseq(keywords[ARROW], (exp_t *)(p)))

/* Analyze the syntax of a cond expression. */
static evproc_t *
ancond(exp_t *ep)
{
        exp_t *cl, *clauses;
        int argc;
        void **argv;

        argc = 1;
        for (clauses = cdr(ep); ispair(clauses); clauses = cdr(clauses)) {
                argc += 2;
                if (!ispair(cl = car(clauses)) || !ispair(cdr(cl)))
                        anerr("should be a test-value pair", cl);
                if (iselse(car(cl)) && !isnull(cdr(clauses)))
                        anerr("else clause must be last", ep);
                if (isarrow(cadr(cl)))
                        if (iselse(car(cl)))
                                anerr("illegal use of arrow", cl);
                        else if (isnull(cddr(cl)) || !isnull(cdddr(cl)))
                                anerr("bad clause form", cl);
                        else
                                ++argc;
        }
        if (!isnull(clauses))
                anerr("should be a list", ep);

        argv = smalloc(argc*sizeof(*argv));
        argv[0] = (void *)argc;
        argc = 1;
        for (clauses = cdr(ep); ispair(clauses); clauses = cdr(clauses)) {
                cl = car(clauses);
                argv[argc++] = iselse(car(cl))?keywords[ELSE]:analyze(car(cl));
                if (isarrow(cadr(cl))) {
                        argv[argc++] = keywords[ARROW];
                        argv[argc++] = analyze(caddr(cl));
                } else
                        argv[argc++] = analyze(nseq(cdr(cl)));
        }

        return nevproc(evcond, argv);
}

/* Analyze the syntax of a set! expression. */
static evproc_t *
anset(exp_t *ep)
{
        void **argv;
        exp_t *var;

        chklst(ep, 3);
        if (!issym(var = cadr(ep)))
                anerr("should be a symbol", var);
        argv = smalloc(2*sizeof(*argv));
        argv[0] = var;
        argv[1] = analyze(caddr(ep));

        return nevproc(evset, argv);
}

/* Analyze the syntax of a set-car! or set-cdr! expression. */
static evproc_t *
ansetpair(exp_t *ep, enum place pl)
{
        evproc_t **argv;

        chklst(ep, 3);
        argv = smalloc(3*sizeof(*argv));
        argv[0] = (evproc_t *)pl;
        argv[1] = analyze(cadr(ep));
        argv[2] = analyze(caddr(ep));

        return nevproc(evsetpair, (void **)argv);
}

/* Analyze the syntax of an `or' or an `and' expression. */
static evproc_t *
anlogic(exp_t *ep, enum logic lg)
{
        evproc_t **argv;
        register int argc;
        exp_t *p;

        ep = cdr(ep);
        for (argc = 1, p = ep; ispair(p); p = cdr(p))
                ++argc;
        if (!isnull(p))
                anerr("should be list", ep);

        argv = smalloc(argc*sizeof(*argv));
        argv[0] = (evproc_t *)argc;
        for (argc = 1; ispair(ep); ep = cdr(ep))
                argv[argc++] = analyze(car(ep));

        return nevproc((lg == LOR ? evor : evand), (void **)argv);
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

#define valerr(var)	raise(&eval_error, filename, linenum,                  \
                              "the expression assigned to %s returns no value",\
                              var)

/* Evaluate a define expression */
static exp_t *
evdef(void **argv, env_t *envp)
{
        symb_t *var;
        evproc_t *vproc;
        exp_t *val;

        var = (symb_t *)argv[0];
        vproc = (evproc_t *)argv[1];
	if (!(val = EVPROC(vproc, envp)))
                valerr(var);
        if (type(val) == PROC && label(val) == NULL)
                label(val) = strtoatm(var); /* label anonymous procedure */
        install(var, val, envp);

        return NULL;
}

/* Evaluate a set! expression. */
static exp_t *
evset(void **argv, env_t *envp)
{
        exp_t *var, *val;
        struct nlist *np;

        var = argv[0];
        if (!(val = EVPROC((evproc_t *)argv[1], envp)))
                valerr(symp(var));
        if (!(np = lookup(symp(var), envp)))
                everr("unbound variable", var);
        np->defn = val;
        return NULL;
}

/* Set the car or the cdr of an expression to a new value. */
static exp_t *
evsetpair(evproc_t **argv, env_t *envp)
{
        exp_t *var, *val;

        if (!ispair(var = EVPROC(argv[1], envp)))
                everr("should be a pair", var);
        if (!(val = EVPROC(argv[2], envp)))
                valerr(symp(var));
        if ((enum place)argv[0] == CAR)
                car(var) = val;
        else
                cdr(var) = val;
        return NULL;
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
        register int i;

        for (i = 1; i < (int)argv[0]-1; i++)
                EVPROC(argv[i], envp);
        return EVPROC(argv[i], envp);
}

/* Evaluate a cond expression */
static exp_t *
evcond(evproc_t **argv, env_t *envp)
{
        int i, argc;
        exp_t *b;

        argc = (int)argv[0];
        for (i = 1; i < argc; i += 2)
                if (iselse(argv[i]) || !iseq(false, b = EVPROC(argv[i], envp)))
                        return isarrow(argv[i+1]) ?
                                apply(EVPROC(argv[i+2], envp), cons(b, null)) :
                                EVPROC(argv[i+1], envp);
                else if (isarrow(argv[i+1]))
                        ++i;

        return null;
}

/* Evaluate an `and' expression */
static exp_t *
evand(evproc_t **argv, env_t *envp)
{
        register int i;
        exp_t *pred;

        for (pred = true, i = 1; i<(int)argv[0] && !iseq(false, pred); i++)
                pred = EVPROC(argv[i], envp);
        return pred;
}

/* Evaluate an `or' expression */
static exp_t *
evor(evproc_t **argv, env_t *envp)
{
        register int i;
        exp_t *pred;

        for (pred = false, i = 1; i<(int)argv[0] && iseq(false, pred); i++)
                pred = EVPROC(argv[i], envp);
        return pred;
}

/* Evaluate a lambda expression */
static exp_t *
evlambda(void **argv, env_t *envp)
{
        return nproc(nfunc((exp_t *)argv[0], (evproc_t *)argv[1], envp));
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
        return apply(EVPROC(argv[1], envp), nreverse(args));
}
