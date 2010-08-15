#include "extern.h"
#include "exp.h"
#include "env.h"
#include "eval.h"
#include "type.h"

const excpt_t eval_error = { "eval" };
const excpt_t syntax_error = { "syntax" };
static enum place { CAR, CDR };
static enum logic { LAND, LOR };

static exp_t *evself(exp_t *, env_t *);
static exp_t *evvar(exp_t *, env_t *);
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
static exp_t *evproc(evproc_t **, env_t *);
static exp_t *evlet(evproc_t **, env_t *);

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
static evproc_t *anlet(exp_t *);

/*
 * Check the syntax of the expression and return a corresponding
 * evaluation procedure.
 */
static evproc_t *
analyze(exp_t *ep)
{
        if (isself(ep))
                return nevproc(evself, ep);
        else if (isvar(ep))
                return nevproc(evvar, ep);
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
        else if (islet(ep))
                return anlet(ep);
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

#define push(x, lst)	((lst) = cons(x, lst))

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
                push(cons(car(pars), car(args)), binds);
        }
        if (!isnull(pars))      /* variable length arguments */
                push(cons(pars, args), binds);
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

/* Analyze the syntax of a quoted expression. */
static evproc_t *
anquote(exp_t *ep)
{
        chklst(ep, 2);
        return nevproc(evself, cadr(ep));
}

static inline void bind(exp_t **, exp_t **, exp_t *);

/* Analyze the syntax of a define expression. */
static evproc_t *
andef(exp_t *ep)
{
        exp_t *var, *val;
        void **argv;

        bind(&var, &val, ep);
        argv = smalloc(2*sizeof(*argv));
        argv[0] = (void *)symp(var);
        argv[1] = (void *)analyze(val);

        return nevproc(evdef, argv);
}

#define nlambda(pars, body)	(cons(keywords[LAMBDA], cons(pars, body)))

/* Bind the variable and the value of a define expression. */
static inline void
bind(exp_t **varp, exp_t **valp, exp_t *lst)
{
        exp_t *ep;

        if (isnull(cdr(lst)) || isatom(cadr(lst)))
                chklst(lst, 3);
        if (ispair(ep = cadr(lst))) { /* lambda shortcut */
                if (!issym(car(ep)))
                        anerr("should be a symbol", car(ep));
                *varp = car(ep);
                *valp = nlambda(cdr(ep), cddr(lst));
        } else if (issym(ep)) {
                *varp = ep;
                *valp = caddr(lst);
        } else
                anerr("the expression couldn't be defined", ep);
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

        return nevproc(evif, argv);
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
        for (argc = 0, lp = cdr(ep); ispair(lp); lp = cdr(lp))
                argv[argc++] = analyze(car(lp));
        argv[argc] = NULL;

        return nevproc(evbegin, argv);
}

#define nseq(ep)           (cons(keywords[BEGIN], ep))
#define nset(var, val)	   (cons(keywords[SET], cons(var, cons(val, null))))
#define nlet(binds, body)  (cons(keywords[LET], cons(binds, body)))
#define nquote(exp)        (cons(keywords[QUOTE], cons(exp, null)))

static void scan_defs(exp_t **, exp_t **, exp_t **, exp_t *);

/* Analyze the syntax of a lambda expression.
 * Make internal definitions simultaneous by transforming
 *    (lambda <vars>
 *      (define u <e1>)
 *      (define v <e2>)
 *      <e3>)
 * into the following procedure
 *    (lambda <vars>
 *      (let ((u '*unassigned*)
 *            (v '*unassigned*))
 *        (set! u <e1>)
 *        (set! v <e2>)
 *        <e3>))
 */
static evproc_t *
anlambda(exp_t *ep)
{
        void **argv;
        exp_t *lp, *p, *vars, *vals, *body;

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

        /* Make the internal definitions simultaneous. */
        scan_defs(&vars, &vals, &body, cddr(ep));
        if (!isnull(vars)) {
                exp_t *binds, *v;
                body = nreverse(body);
                for (v = vars; !isnull(v); v = cdr(v)) {
                        push(nset(car(v), car(vals)), body);
                        vals = cdr(vals);
                }
                for (binds = null; !isnull(vars); vars = cdr(vars))
                        push(cons(car(vars),
                                  cons(nquote(undefined), null)),
                             binds);
                cddr(ep) = cons(nlet(binds, body), null);
        }

        argv = smalloc(2*sizeof(*argv));
        argv[0] = (void *)cadr(ep);
        argv[1] = (void *)anbegin(nseq(cddr(ep)));

        return nevproc(evlambda, argv);
}

/*
 * Bind the variables and values of the definitions in ep to *varsp
 * and *valsp, and the remaining body to *bodyp
 */
static void
scan_defs(exp_t **varsp, exp_t **valsp, exp_t **bodyp, exp_t *ep)
{
        exp_t *var, *val, *body;

        *varsp = *valsp = *bodyp = null;
        for (body = ep; ispair(body); body = cdr(body))
                if (isdef(car(body))) {
                        bind(&var, &val, car(body));
                        push(var, *varsp);
                        push(val, *valsp);
                } else
                        push(car(body), *bodyp);
        if (!isnull(body))
                anerr("should be a list", ep);
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
        argc = 0;
        for (clauses = cdr(ep); ispair(clauses); clauses = cdr(clauses)) {
                cl = car(clauses);
                argv[argc++] = iselse(car(cl))?keywords[ELSE]:analyze(car(cl));
                if (isarrow(cadr(cl))) {
                        argv[argc++] = keywords[ARROW];
                        argv[argc++] = analyze(caddr(cl));
                } else
                        argv[argc++] = analyze(nseq(cdr(cl)));
        }
        argv[argc] = NULL;

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

        return nevproc(evsetpair, argv);
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
        for (argc = 0; ispair(ep); ep = cdr(ep))
                argv[argc++] = analyze(car(ep));
        argv[argc] = NULL;

        return nevproc((lg == LOR ? evor : evand), argv);
}

/* Analyze the syntax of a `let' expression. */
static evproc_t *
anlet(exp_t *ep)
{
        evproc_t **argv;
        exp_t *bd, *binds, *body, *name, *op, *pars, *vals;

        if (isnull(cdr(ep)))
                anerr("bad syntax", ep);
        if (issym(cadr(ep))) {  /* named let */
                name = cadr(ep);
                if (isnull(cddr(ep)))
                        anerr("bad syntax", ep);
                binds = caddr(ep);
                body = cdddr(ep);
        } else {
                name = NULL;
                binds = cadr(ep);
                body = cddr(ep);
        }

        for (pars = vals = null; ispair(binds); binds = cdr(binds))
                if (issym(bd = car(binds))) {
                        push(bd, pars);
                        push(null, vals);
                } else if (ispair(bd) && ispair(cdr(bd)) && isnull(cddr(bd))) {
                        push(car(bd), pars);
                        push(cadr(bd), vals);
                } else
                        anerr("bad binding syntax", ep);
        if (!isnull(binds))
                anerr("should be a list of bindings", binds);

        argv = smalloc(2*sizeof(*argv));
        op = nlambda(nreverse(pars), body);
        if (name) {             /* named let */
                argv[0] = analyze(cons(keywords[DEFINE],
                                       cons(name, cons(op, null))));
                op = name;
        } else
                argv[0] = NULL;
        argv[1] = analyze(cons(op, nreverse(vals)));

        return nevproc(evlet, argv);
}

/* Analyze the syntax of an application expression. */
static evproc_t *
anapp(exp_t *ep)
{
        void **argv;
        exp_t *p;
        register int argc;

        for (argc = 1, p = ep; ispair(p); p = cdr(p))
                ++argc;
        if (!isnull(p))
                anerr("an application should be a list, given", ep);
        argv = smalloc(argc*sizeof(*argv));
        for (argc = 0, p = ep; ispair(p); p = cdr(p))
                argv[argc++] = analyze(car(p));
        argv[argc] = NULL;

        return nevproc(evapp, argv);
}

/* * * * * * * * * * * * * *
 * Evaluation procedures.  *
 * * * * * * * * * * * * * */

static exp_t *
evself(exp_t *ep, env_t *envp)
{
        return ep;
}

/* Return the value of a variable if any. */
static exp_t *
evvar(exp_t *var, env_t *envp)
{
        struct nlist *np;

        if (!(np = lookup(symp(var), envp)) || np->defn == undefined)
                everr("unbound variable", var);
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
        for (; *(argv+1); argv++)
                EVPROC(argv[0], envp);
        return EVPROC(argv[0], envp);
}

/* Evaluate a cond expression */
static exp_t *
evcond(evproc_t **argv, env_t *envp)
{
        exp_t *b;

        for (; *argv; argv += 2)
                if (iselse(*argv) || !iseq(false, b = EVPROC(*argv, envp)))
                        return isarrow(*(argv+1)) ?
                                apply(EVPROC(*(argv+2), envp), cons(b, null)) :
                                EVPROC(*(argv+1), envp);
                else if (isarrow(*(argv+1)))
                        ++argv;

        return null;
}

/* Evaluate an `and' expression */
static exp_t *
evand(evproc_t **argv, env_t *envp)
{
        exp_t *pred;

        for (pred = true; *argv && !iseq(false, pred); argv++)
                pred = EVPROC(*argv, envp);
        return pred;
}

/* Evaluate an `or' expression */
static exp_t *
evor(evproc_t **argv, env_t *envp)
{
        exp_t *pred;

        for (pred = false; *argv && iseq(false, pred); argv++)
                pred = EVPROC(*argv, envp);
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
evlet(evproc_t **argv, env_t *envp)
{
        if (argv[0])           /* named let */
                EVPROC(argv[0], envp);
        return EVPROC(argv[1], envp);
}

/* Evaluate an application expression. */
static exp_t *
evapp(evproc_t **argv, env_t *envp)
{
        evproc_t *op;
        exp_t *args;

        op = *argv++;
        for (args = null; *argv; argv++)
                push(EVPROC(*argv, envp), args);
        return apply(EVPROC(op, envp), nreverse(args));
}
