#ifndef EVAL_H
#define EVAL_H

extern const excpt_t eval_error;
extern const excpt_t syntax_error;

extern exp_t *eval(exp_t *, env_t *);
extern exp_t *apply(exp_t *, exp_t *);

#define everr(msg, ep)	RAISE1(eval_error, msg" %s", tostr(ep))
#define anerr(msg, ep)  RAISE1(syntax_error, msg" %s", tostr(ep))

#endif /* !EVAL_H */
