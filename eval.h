#ifndef EVAL_H
#define EVAL_H

extern const excpt_t eval_error;
extern const excpt_t syntax_error;

extern exp_t *eval(exp_t *, env_t *);
extern exp_t *apply(exp_t *, exp_t *);

#define everr(msg, ep)	raise(&eval_error, instream->name, instream->line, \
                              instream->col, msg" %s", tostr(ep))
#define anerr(msg, ep)  raise(&syntax_error, instream->name, instream->line, \
                              instream->col, msg" %s", tostr(ep))

#endif /* !EVAL_H */
