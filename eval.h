#ifndef EVAL_H
#define EVAL_H

extern const excpt_t eval_error;
extern exp_t *eval(exp_t *, env_t *);
extern exp_t *apply(exp_t *, exp_t *, env_t *);

/* Print the message and return a null pointer */
static inline exp_t *
everr(const char *msg, const exp_t *ep)
{
        char *s;

        warnx("%s: %s", msg, s = tostr(ep));
        free(s);
        return NULL;
}
#endif /* !EVAL_H */
