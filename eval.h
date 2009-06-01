#ifndef EVAL_H
#define EVAL_H

struct exp *eval(struct exp *, struct env *);


/* Print the message and return a null pointer */
static __inline__ struct exp *
everr(char *msg, struct exp *ep)
{
  char *s;
  
  warnx("%s: %s", msg, s = tostr(ep));
  free(s);
  return NULL;
}
#endif /* !EVAL_H */
