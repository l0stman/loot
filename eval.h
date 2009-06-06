#ifndef EVAL_H
#define EVAL_H

exp_t *eval(exp_t *, env_t *);

/* Print the message and return a null pointer */
static inline exp_t *
everr(char *msg, exp_t *ep)
{
  char *s;
  
  warnx("%s: %s", msg, s = tostr(ep));
  free(s);
  return NULL;
}
#endif /* !EVAL_H */
